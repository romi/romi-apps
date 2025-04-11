#ifndef _ROMI_ILOGWRITER_H
#define _ROMI_ILOGWRITER_H

#include <filesystem>

namespace romi
{
        class ILogWriter
        {
        public:
                ILogWriter() = default;
                virtual ~ILogWriter() = default;
                virtual void open(std::string_view name) = 0;
                virtual void close() = 0;
                virtual void write(const std::string& message) = 0;
        };

        class ILogWriterFactory
        {
        public:
                ILogWriterFactory() = default;
                virtual ~ILogWriterFactory() = default;
                virtual std::shared_ptr<ILogWriter> create_console_writer() = 0;
                virtual std::shared_ptr<ILogWriter> create_file_writer() = 0;
        };
}

#endif
