/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <chrono>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include "util/ISubprocess.h"

namespace romi {

        class Subprocess  : publuc ISubprocess
        {
        public:

                explicit Subprocess()
                        : cmd_(),
                          new_pgrp_(true)
                {}

                // Non-copyable
                Subprocess(const Subprocess&) = delete;
                Subprocess& operator=(const Subprocess&) = delete;

                // Not movable
                Subprocess(Subprocess&& other) = delete;
                Subprocess& operator=(Subprocess&& other) = delete;

                ~Subprocess() {
                        // Best-effort: if the child already exited,
                        // reap it to avoid zombies.
                        cleanup_non_blocking();
                }

                // Start the subprocess. commandLine is executed as:
                // /bin/sh -c "<commandLine>" If newProcessGroup=true,
                // we create a new process group for the child so you
                // can signal the whole group (pipelines, children)
                // via kill(-pgid, sig). Returns true on success;
                // throws on error by default.
                void start(std::string commandLine, bool newProcessGroup = true) override {
                        cmd_ = commandLine;
                        new_pgrp_ = newProcessGroup;
                        
                        if (pid_ > 0) {
                                throw std::runtime_error("Subprocess already started");
                        }

                        pid_t p = ::fork();
                        if (p < 0) {
                                throw std::runtime_error(std::string("fork failed: ")
                                                         + std::strerror(errno));
                        }

                        if (p == 0) {
                                // Child process
                                if (new_pgrp_) {
                                        // Make the child the leader
                                        // of a new process group.
                                        // This lets the parent
                                        // kill/signal the entire
                                        // group.
                                        (void)::setpgid(0, 0);
                                }

                                // Exec command line through the shell
                                // to support pipes/redirection/etc.
                                ::execl("/bin/sh", "sh", "-c",
                                        cmd_.c_str(),
                                        (char*) nullptr);

                                // If execl returns, it failed.
                                _exit(127);
                        }

                        // Parent process
                        pid_ = p;

                        if (new_pgrp_) {
                                // Try to set pgid from parent too
                                // (handles race; ignore if already
                                // set).
                                if (::setpgid(pid_, pid_) == 0) {
                                        pgid_ = pid_;
                                } else {
                                        // Even if setpgid fails due
                                        // to race/EACCES, the child
                                        // likely set it.  We'll fall
                                        // back to using pid as pgid
                                        // when signaling groups.
                                        pgid_ = pid_;
                                }
                        }
                }

                // Returns true if process appears to still be
                // running. If the child has exited, this will reap it
                // and store the result internally.
                bool is_running() override {
                        if (pid_ <= 0) {
                                return false;
                        }
                        
                        int status = 0;
                        pid_t r = ::waitpid(pid_, &status, WNOHANG);
                        
                        if (r == 0) {
                                // still running
                                return true;
                        }
                        if (r == pid_) {
                                // exited
                                result_ = decode_status(status);
                                pid_ = -1;
                                return false;
                        }
                        // r < 0
                        if (errno == EINTR) {
                                // transient; assume running
                                return true;
                        }
                        if (errno == ECHILD) {
                                // already reaped elsewhere
                                pid_ = -1;
                                return false;
                        }
                        throw std::runtime_error(std::string("waitpid(WNOHANG) failed: ")
                                                 + std::strerror(errno));
                }

                // Wait until the process exits. Returns decoded
                // result.  If already exited, returns cached result
                // if available.
                Result wait() override {
                        if (pid_ <= 0) {
                                if (result_) {
                                        return result_;
                                }
                                // Not running and no cached result: treat as "unknown".
                                return Result{};
                        }

                        int status = 0;
                        while (true) {
                                pid_t r = ::waitpid(pid_, &status, 0);
                                if (r == pid_) {
                                        break;
                                }
                                if (r < 0 && errno == EINTR) {
                                        continue;
                                }
                                throw std::runtime_error(std::string("waitpid failed: ")
                                                         + std::strerror(errno));
                        }

                        result_ = decode_status(status);
                        pid_ = -1;
                        return result_;
                }

                // Convenience: try graceful stop with SIGTERM, then
                // SIGKILL after timeout.  Returns true if it is no
                // longer running at the end.
                bool stop(std::chrono::milliseconds timeout,
                          bool toProcessGroup = true,
                          int gracefulSig = SIGTERM,
                          int forceSig = SIGKILL) override {
                        
                        if (pid_ <= 0)
                                return true;

                        // Graceful
                        send_signal(gracefulSig, toProcessGroup);

                        auto deadline = std::chrono::steady_clock::now() + timeout;
                        while (std::chrono::steady_clock::now() < deadline) {
                                if (!is_running()) {
                                        return true;
                                }
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }

                        // Force
                        send_signal(forceSig, toProcessGroup);
                        
                        // Reap (blocking)
                        wait();
                        return true;
                }

                Result last_result() const override {
                        return result_;
                }

        private:

                // Send a signal to the child or to its process group.
                // If toProcessGroup=true and newProcessGroup was
                // enabled, we signal the group.  Returns true if
                // kill() succeeded, false if process doesn't exist
                // (ESRCH).
                bool send_signal(int sig, bool toProcessGroup = true) const {
                        if (pid_ <= 0)
                                return false;

                        pid_t target = pid_;
                        if (toProcessGroup && new_pgrp_) {
                                // Negative PID => signal process group.
                                target = -pgid_;
                        }

                        if (::kill(target, sig) == 0) {
                                return true;
                        }
                        if (errno == ESRCH) {
                                return false;
                        }
                        throw std::runtime_error(std::string("kill failed: ")
                                                 + std::strerror(errno));
                }

                pid_t pid() const {
                        return pid_;
                }
                
                pid_t pgid() const {
                        return pgid_;
                }
                
                static Result decode_status(int status) {
                        Result r;
                        r.status = status;
                        if (WIFEXITED(status)) {
                                r.exit_code = WEXITSTATUS(status);
                        } else if (WIFSIGNALED(status)) {
                                r.term_signal = WTERMSIG(status);
                        }
                        return r;
                }

                void cleanup_non_blocking() {
                        if (pid_ <= 0) {
                                return;
                        }
                        int status = 0;
                        pid_t r = ::waitpid(pid_, &status, WNOHANG);
                        if (r == pid_) {
                                result_ = decode_status(status);
                                pid_ = -1;
                        }
                        // If still running (r==0), leave it alone (caller owns lifecycle).
                }

        private:
                std::string cmd_;
                bool new_pgrp_ = true;
                pid_t pid_ = -1;
                pid_t pgid_ = -1; // only meaningful if new_pgrp_==true
                Result result_;
        };
}
