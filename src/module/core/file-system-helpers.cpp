#include "file-system-helpers.h"

#include <boost/filesystem.hpp>
#include <memory>
#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {

    std::string GetFileFullPath(const std::string& file) {
        return boost::filesystem::absolute(file).string();
    }

}   // End of anonymous namespace.

std::string file_system_helpers::ReadFileSync(const std::string& filename) {
    std::string fileFullPath = GetFileFullPath(filename);

    FILE* source = nullptr;
    fopen_s(&source, fileFullPath.c_str(), "rb");
    if (source == nullptr) {
        std::ostringstream oss;
        oss << "Can't open for read " << fileFullPath;
        throw std::runtime_error(oss.str());
    }

    std::unique_ptr<FILE, std::function<void(FILE*)>> deferred(source, [](auto file) {
        fclose(file);
    });

    fseek(source, 0, SEEK_END);
    auto size = static_cast<size_t>(ftell(source));
    rewind(source);

    std::string content;
    content.resize(size);
    //content[size] = '\0';

    for (size_t i = 0; i < size; ) {
        i += fread(&content[i], 1, size - i, source);
        if (ferror(source) != 0) {
            std::ostringstream oss;
            oss << "Can't read " << fileFullPath;
            throw std::runtime_error(oss.str());
        }
    }

    return content;
}

void file_system_helpers::WriteFileSync(const std::string& filename, const char* data, size_t length) {
    auto fileFullPath = GetFileFullPath(filename);
    
    FILE* target = nullptr;
    fopen_s(&target, fileFullPath.c_str(), "wb");
    if (target == nullptr) {
        std::ostringstream oss;
        oss << "Can't open for write " << fileFullPath;
        throw std::runtime_error(oss.str());
    }

    std::unique_ptr<FILE, std::function<void(FILE*)>> deferred(target, [](auto file) {
        fclose(file);
    });

    for (size_t written = 0; written < length; ) {
        written += fwrite(data + written, 1, length - written, target);
        if (ferror(target) != 0) {
            std::ostringstream oss;
            oss << "Can't write " << fileFullPath;
            throw std::runtime_error(oss.str());
        }
    }
}

void file_system_helpers::MkdirSync(const std::string& directory) {
    boost::filesystem::path path(GetFileFullPath(directory));
    if (!boost::filesystem::exists(path) && !boost::filesystem::create_directory(path))
    {
        std::ostringstream oss;
        oss << "The directory: " << directory << " doesn't exist, and can't be created.";
        throw std::runtime_error(oss.str());
    }
}

bool file_system_helpers::ExistsSync(const std::string& path) {
    auto fullPath = GetFileFullPath(path);
    return boost::filesystem::exists(fullPath);
}