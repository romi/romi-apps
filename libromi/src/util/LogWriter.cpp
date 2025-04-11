#include "util/LogWriter.h"

namespace romi {

        std::shared_ptr<ILogWriter> LogWriterFactory::create_console_writer()
        {
                return std::make_shared<ConsoleLogWriter>();
        }

        std::shared_ptr<ILogWriter> LogWriterFactory::create_file_writer()
        {
                return std::make_shared<FileLogWriter>();
        }

        FileLogWriter::FileLogWriter() : write_stream()
        {
        }

        void FileLogWriter::open(const std::string_view file_name)
        {
                close();
                write_stream.open(std::filesystem::path(file_name),
                                  std::ios_base::binary|std::ios_base::out|std::ios_base::app);
        }

        void FileLogWriter::close()
        {
                if (write_stream.is_open()) {
                        write_stream.close();
                }
        }

        void FileLogWriter::write(const std::string &message)
        {
                write_stream << message << std::flush;
        }
}
