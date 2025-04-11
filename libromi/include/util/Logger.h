#ifndef ROMI_LOGGER_H
#define ROMI_LOGGER_H

#include <mutex>
#include <fstream>
#include <map>
#include "util/ILogWriter.h"
#include "util/ILogger.h"
#include "util/StringUtils.h"

namespace romi
{

        class Logger : public ILogger
        {
        private:
                static std::string filename_;
                std::string application_name_;
                static std::recursive_mutex log_mutex_;
                static std::shared_ptr<ILogger> logger_;
                std::map<log_level, std::string> log_level_names_;
                const std::shared_ptr<ILogWriterFactory> logWriterFactory_;
                std::shared_ptr<ILogWriter> logWriter_;

        private:
                explicit Logger(const std::shared_ptr<ILogWriterFactory>& logWriterFactory);

        public:
                ~Logger() override = default;
                Logger(Logger &other) = delete;
                void operator=(const Logger &) = delete;
                static std::shared_ptr<ILogger> Instance();
                void move_log(std::filesystem::path newpath) override;
                void log(log_level level, const char* format, ...) override;
                std::string get_log_file_path() override;
                void set_application_name(std::string_view application_name) override;
                void log_to_file(const std::string &log_path) override;
                void log_to_console() override;
        public:
                // The only real way to test a singleton with
                // Dependency injection. Best of all evils.
                friend void set_instance(const std::shared_ptr<ILogWriterFactory>& factory);
                friend void clear_instance();

        };
}

// Outside of namespace for now as we are replacing C functions.  This
// enables us to not change the whole code base, whilst using a more
// testable C++ class under the hood.

int log_init();
void log_cleanup();
void log_set_application(std::string_view application_name);
int log_set_file(const std::string &path);
std::string log_get_file();
void log_set_console();
void log_move(std::filesystem::path newpath);

template <typename ...Args>
void r_err(const std::string format, Args && ...args) {
        romi::Logger::Instance()->log(romi::log_level::ERROR, format.c_str(),
                                      std::forward<Args>(args)...);
}

template <typename ...Args>
void r_warn(const std::string format, Args && ...args){
        romi::Logger::Instance()->log(romi::log_level::WARNING, format.c_str(),
                                      std::forward<Args>(args)...);
}

template <typename ...Args>
void r_info(const std::string format, Args && ...args){
        romi::Logger::Instance()->log(romi::log_level::INFO, format.c_str(),
                                      std::forward<Args>(args)...);
}

template <typename ...Args>
void r_debug(const std::string format, Args && ...args) {
        romi::Logger::Instance()->log(romi::log_level::DEBUG, format.c_str(),
                                      std::forward<Args>(args)...);
}

#endif // ROMI_LOGGER_H
