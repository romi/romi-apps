#include <iostream>
#include <mutex>
#include "util/Logger.h"
#include "util/LogWriter.h"
#include "util/ClockAccessor.h"

namespace romi {
        
        std::recursive_mutex Logger::log_mutex_;
        std::string Logger::filename_;
        std::shared_ptr<ILogger> Logger::logger_;

        std::shared_ptr<ILogger> Logger::Instance() {
                {
                        std::scoped_lock lock(log_mutex_);
                        if (logger_ == nullptr) {
                                logger_ = std::shared_ptr<Logger>(new Logger(std::make_shared<LogWriterFactory>()));
                        }
                }
                return logger_;
        }

        Logger::Logger(const std::shared_ptr<ILogWriterFactory>& logWriterFactory)
                : application_name_("??"),
                  log_level_names_(),
                  logWriterFactory_(logWriterFactory),
                  logWriter_()
        {
                logWriter_ = logWriterFactory->create_console_writer();
                log_level_names_[log_level::DEBUG] = "DD";
                log_level_names_[log_level::INFO] = "II";
                log_level_names_[log_level::WARNING] = "WW";
                log_level_names_[log_level::ERROR] = "EE";
        }

        void Logger::move_log(std::filesystem::path newpath)
        {
                std::scoped_lock lock(log_mutex_);
                std::filesystem::path current_log_path;
                std::string new_filename = "log.txt";

                if (!filename_.empty()) {
                        current_log_path = filename_;
                        new_filename = current_log_path.filename();
                }
                newpath /= new_filename;

                try {
                        logWriter_->close();
                        {
                                if (!current_log_path.empty())
                                        std::filesystem::rename(current_log_path, newpath);
                        }
                        log_set_file(newpath.string());
                } catch (std::filesystem::filesystem_error& e) {
                        std::cout << "move_log() failed to move " << current_log_path << " to " << newpath << std::endl;
                        std::cout << e.what() << std::endl;
                        throw;
                }
        }

        // This can't be a variadic template due to wanting it in the
        // interface base class, so we use the old ... notation.
        void Logger::log(log_level level, const char* format, ...)
        {
                std::stringstream logger_stream;
                std::string log_level = log_level_names_[level];

                std::string message;
                va_list argptr;
                va_start(argptr, format);
                StringUtils::string_vprintf(message, format, argptr);
                va_end(argptr);

                logger_stream
                        << ClockAccessor::GetInstance()->datetime_compact_string()
                        << ", "
                        << log_level << ", " << application_name_
                        << ", 0x" << std::hex << pthread_self()
                        << std::dec << ", "
                        << message << std::endl;
                {
                        std::scoped_lock lock(log_mutex_);
                        logWriter_->write(logger_stream.str());
                }
        }

        std::string Logger::get_log_file_path()
        {
                return filename_;
        }

        void Logger::log_to_file(const std::string &log_path)
        {
                std::scoped_lock lock(log_mutex_);
                filename_ = log_path;
                log(log_level::INFO, "Changing log to '%s'", log_path.c_str());
                logWriter_->close();
                logWriter_ = logWriterFactory_->create_file_writer();
                logWriter_->open(log_path);
        }

        void Logger::set_application_name(std::string_view application_name)
        {
                std::scoped_lock lock(log_mutex_);
                application_name_ = application_name;
        }

        void Logger::log_to_console()
        {
                std::scoped_lock lock(log_mutex_);
                filename_ = "";
                log(log_level::INFO, "Changing log to console");
                logWriter_->close();
                logWriter_ = logWriterFactory_->create_console_writer();
        }


} // namespace

// TBD - to remove..
int log_init()
{
        return 0;
}

// TBD - to remove..
void log_cleanup()
{
        romi::Logger::Instance()->log_to_console();
}

void log_set_application(std::string_view application_name)
{
        romi::Logger::Instance()->set_application_name(application_name);
}

int log_set_file(const std::string &path)
{
        romi::Logger::Instance()->log_to_file(path);
        return 0;
}

std::string log_get_file()
{
        return romi::Logger::Instance()->get_log_file_path();
}

void log_set_console()
{
        romi::Logger::Instance()->log_to_console();
}

void log_move(std::filesystem::path newpath)
{
        romi::Logger::Instance()->move_log(newpath);
}
