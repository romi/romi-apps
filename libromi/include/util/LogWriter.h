#ifndef _ROMI_LOGWRITER_H
#define _ROMI_LOGWRITER_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include "util/ILogWriter.h"

namespace romi {
        
        class ConsoleLogWriter : public ILogWriter
        {
        public:
                ConsoleLogWriter() = default;
                ~ConsoleLogWriter() override = default;
                void open(const std::string_view name) override {(void)name;};
                void close() override {};
                void write(const std::string& message) override{
                        std::cout << message << std::flush;
                };
        };

        class FileLogWriter : public ILogWriter
        {
        private:
                std::ofstream write_stream;
        public:
                FileLogWriter();
                ~FileLogWriter() override = default;
                void open(std::string_view file_name) override;
                void close() override;
                void write(const std::string& message) override;
        };

        class LogWriterFactory : public ILogWriterFactory
        {
        public:
                LogWriterFactory() = default;
                ~LogWriterFactory() override = default;
                std::shared_ptr<ILogWriter> create_console_writer() override;
                std::shared_ptr<ILogWriter> create_file_writer() override;
        };
}

#endif
