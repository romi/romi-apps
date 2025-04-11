//#include "ILinux.h"
#include "util/FileUtils.h"

namespace romi {
        
        void FileUtils::TryReadFileAsVector(const std::string &filename,
                                            std::vector <uint8_t> &out)
        {
                try {
                        std::ifstream in(filename, std::ios::in | std::ios::binary);
                        in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                        std::copy((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>(),
                                  std::back_inserter(out));
                } catch (const std::istream::failure &ex) {
                        FILE_UTILS_EXCEPTION_LOG("\"" << filename.c_str()
                                                 << "\"" << " " << ex.what())
                                throw;
                }
        }

        void FileUtils::TryWriteVectorAsFile(const std::string& filename,
                                             const std::vector<uint8_t>& in)
        {
                try {
                        std::ofstream out(filename, std::ios::out | std::ios::binary);
                        out.exceptions(std::ofstream::badbit | std::ofstream::failbit);
                        std::copy(in.begin(), in.end(),
                                  std::ostream_iterator<uint8_t>(out));
                } catch (const std::ostream::failure& ex) {
                        FILE_UTILS_EXCEPTION_LOG( "\"" << filename.c_str() << "\""
                                                  << " " << ex.what())
                                throw;
                }
        }

        std::string FileUtils::TryReadFileAsString(const std::string& filename)
        {
                std::string out;
                try {
                        std::ifstream in(filename);
                        in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                        std::copy((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>(),
                                  std::back_inserter(out));
                } catch (const std::exception& ex) {
                        FILE_UTILS_EXCEPTION_LOG( "\"" << filename.c_str() << "\""
                                                  << " " << ex.what())
                                throw;
                }

                return(out);
        }

        void FileUtils::TryWriteStringAsFile(const std::string& filename,
                                             const std::string& output)
        {
                try {
                        std::ofstream out(filename);
                        out.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                        std::copy(output.begin(), output.end(),
                                  std::ostream_iterator<char>(out));
                } catch (const std::exception& ex)  {
                        FILE_UTILS_EXCEPTION_LOG( "\"" << filename.c_str()
                                                  << "\"" << " " << ex.what())
                                throw;
                }
        }

// fs::path FileUtils::TryGetHomeDirectory(rpp::ILinux& linux)
// {
//         try {
//                 char* homedir = nullptr;
//                 homedir = linux.secure_getenv("HOME");
//                 if (homedir == nullptr) {
//                         homedir = linux.getpwuid(linux.getuid())->pw_dir;
//                         if (!homedir)
//                                 throw std::runtime_error("TryGetHomeDirectory() getpwuid() failed.");
//                 }
//                 return fs::path(homedir);
//         } catch (const std::exception& ex) {
//                 FILE_UTILS_EXCEPTION_LOG( "\"" << "failed to get home dir"
//                                           << "\"" << " " << ex.what())
//                         throw;
//         }
// }

}
