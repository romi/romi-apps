#ifndef _ROMI_ILOGGER_H
#define _ROMI_ILOGGER_H

#include <filesystem>

namespace romi {
        
        enum class log_level {
                DEBUG = 1,
                INFO,
                WARNING,
                ERROR
        };

        class ILogger
        {
        public:
                ILogger() = default;
                virtual ~ILogger() = default;
                virtual void log(log_level level, const char* format, ...) = 0;

                virtual std::string get_log_file_path() = 0;
                virtual void set_application_name(std::string_view application_name) = 0;
                virtual void log_to_file(const std::string &log_path) = 0;
                virtual void log_to_console() = 0;
                virtual void move_log(std::filesystem::path newpath) = 0;
        };
}

#endif // _ROMI_ILOGGER_H
