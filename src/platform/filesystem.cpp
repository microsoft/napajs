#include <platform/filesystem.h>
#include <platform/platform.h>

#include "utils/string.h"

#include <algorithm>
#include <cstdio>
#include <deque>
#include <sstream>
#include <iostream>
#include <vector>


namespace napa {
namespace filesystem {
using CharType = Path::CharType;
using StringType = Path::StringType;

namespace {

    /// <summary> Tell if a character is path separator. </summary>
    bool IsSeparator(char ch) {
#ifdef SUPPORT_POSIX
        return ch == '/';
#else
        return ch == '/' || ch == '\\';
#endif
    }

    /// <summary> Tell if a character is a letter. </summary>
    bool IsLetter(char ch) {
        return (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z');
    }

    /// <summary> Tell if it's a root of Windows path. </summary>
    bool IsWindowsRootSegment(const StringType& segment) {
        return segment.size() == 2 && IsLetter(segment[0]) && segment[1] == ':';
    }

    /// <summary> Has UNC prefix "\\\\?" in windows. </summary>
    bool ParseUncPrefix(const std::string& path) {
        if (path.size() < 4) {
            return false;
        }
        return path[0] == '\\' 
            && path[1] == '\\' 
            && path[2] == '?'
            && path[3] == '\\';
    }

    /// <summary> Parse driver spec. </summary>
    std::string ParseDriveSpec(const std::string& path, bool* hasUncPrefix = nullptr) {
        StringType::size_type driveSpecStart = 0;
        auto unc = ParseUncPrefix(path);

        if (unc) {
            driveSpecStart = 4;
        }
        if (path.size() < driveSpecStart + 2 ||
            !IsLetter(path[driveSpecStart]) ||
            path[driveSpecStart + 1] != ':') {
            return "";
        }
        if (hasUncPrefix != nullptr) {
            *hasUncPrefix = unc;
        }
        return path.substr(driveSpecStart, 2);
    }

    /// <summary> Resolve path string into segments. </summary>
    void Parse(
        const StringType& path,
        bool* hasUncPrefix,
        StringType* driveSpec,
        bool* isAbsolute,
        std::vector<StringType>* segments) {

        StringType::size_type pathStart = 0;
        bool unc = false;
        auto drive = ParseDriveSpec(path, &unc);

        if (unc) {
            pathStart += 4;
        }

        if (!drive.empty()) {
            pathStart += 2;
        }

        auto absolute = path.size() > pathStart && IsSeparator(path[pathStart]);

        if (pathStart < path.size() && segments != nullptr) {
            std::vector<StringType> segs;
            std::deque<StringType> resolved;
            utils::string::Split(path.begin() + pathStart, path.end(), segs, StringType("\\/"), true);

            for (auto it = segs.begin(); it < segs.end(); ++it) {
                auto& seg = *it;
                if (seg == "..") {
                    // Backtrace '..'.
                    if (resolved.empty() || resolved.back() == "..") {
                        resolved.push_back(seg);
                    }
                    else {
                        resolved.pop_back();
                    }
                }
                else if (seg != ".") {
                    // Skip '.'
                    resolved.push_back(seg);
                }
            }
            segments->reserve(resolved.size());
            for (auto it = resolved.begin(); it != resolved.end(); ++it) {
                segments->emplace_back(std::move(*it));
            }
        }

        if (hasUncPrefix != nullptr) {
            *hasUncPrefix = unc;
        }

        if (driveSpec != nullptr) {
            *driveSpec = drive;
        }

        if (isAbsolute != nullptr) {
            *isAbsolute = absolute;
        }
    }

    /// <summary> Format path name from path meta-data. </summary>
    std::string FormatPathString(
        bool hasUncPrefix,
        const std::string& driveSpec,
        bool isAbsolute,
        const std::vector<StringType>& segments,
        const CharType* seperator = platform::DIR_SEPARATOR) {

        // Output normalized string.
        std::stringstream ss;

        if (hasUncPrefix) {
            ss << "\\\\?\\";
        }

        // Drive spec could be empty.
        ss << driveSpec;

        if (isAbsolute) {
            ss << platform::DIR_SEPARATOR;
        }

        for (auto it = segments.begin(); it != segments.end(); ++it) {
            if (it != segments.begin()) {
                ss << seperator;
            }
            ss << *it;
        }
        auto path = ss.str();
        if (path.size() == 0) {
            return ".";
        }
        return path;
    }

}

Path& Path::operator=(const CharType* path) {
    _pathname = path;
    return *this;
}

Path& Path::operator=(const StringType& path) {
    _pathname = path;
    return *this;
}

Path& Path::operator=(const Path& path) {
    _pathname = path._pathname;
    return *this;
}

Path& Path::operator=(StringType&& path) {
    _pathname = std::move(path);
    return *this;
}

Path& Path::operator=(Path&& path) {
    _pathname = std::move(path._pathname);
    return *this;
}

Path& Path::operator/=(const Path& path) {
    return Append(path);
}

Path Path::operator/(const Path& path) const {
    Path tmp(*this);
    tmp /= path;
    return tmp;
}

int Path::Compare(const Path& path) const {
    return _pathname.compare(path._pathname);
}

/// <summary> Get unnormalized path string. </summary>
/// <remarks> Use GenericForm() to always use '/' as delimiter. </summary>
const std::string& Path::String() const {
    return _pathname;
}

/// <summary> Get unnormalized path string buffer. </summary>
const CharType* Path::c_str() const {
    return _pathname.c_str();
}

Path& Path::Append(const Path& path) {
    if (path.IsAbsolute()) {
#ifdef _WIN32
        auto drive = path.DriveSpec();
        auto baseDrive = DriveSpec();

        if (!path.HasUncPrefix() && drive.IsEmpty()) {
            _pathname = baseDrive.String() + path._pathname;
            return *this;
        }
#endif
        _pathname = path._pathname;
    }
    else {
        if (!path._pathname.empty()) {
            if (!IsSeparator(path._pathname[0])) {
                _pathname += platform::DIR_SEPARATOR;
            }
            _pathname += path._pathname;
        }
    }
    return *this;
}

Path& Path::Normalize() {
    if (!_pathname.empty()) {
        std::string driveSpec;
        bool hasUncPrefix = false;
        bool isAbsolute = false;
        std::vector<StringType> segs;

        Parse(_pathname, &hasUncPrefix, &driveSpec, &isAbsolute, &segs);

        // No parent path.
        if (isAbsolute && segs.size() > 0 && segs[0] == "..") {
            _pathname = "";
        }
        else {
            _pathname = FormatPathString(hasUncPrefix, driveSpec, isAbsolute, segs, platform::DIR_SEPARATOR);
        }
    }
    return *this;
}

Path& Path::ReplaceExtension(const std::string& extension) {
    auto pos = _pathname.find_last_of("./\\");
    if (pos != StringType::npos && pos != _pathname.size() - 1 && _pathname[pos] == '.') {
        _pathname.replace(pos, _pathname.size(), extension);
    }
    return *this;
}

Path Path::GenericForm() const {
    if (HasUncPrefix()) {
        return *this;
    }

    Path path(*this);
    path.Normalize();

#ifdef _WIN32
    std::replace(path._pathname.begin(), path._pathname.end(), '\\', '/');
#endif
    return path;
}

Path Path::DriveSpec() const {
    return Path(ParseDriveSpec(_pathname));
}

Path Path::Parent() const {
    if (IsEmpty()) {
        return Path();
    }
    return *this / "..";
}

Path Path::Dirname() const {
    if (IsEmpty()) {
        return Path(".");
    }

    auto pos = _pathname.find_last_of("/\\");
    if (pos == StringType::npos) {
        // relative path, return '.' as directory name.
        return Path(".");
    }
    return Path(_pathname.substr(0, pos));
}

Path Path::Extension() const {
    auto pos = _pathname.find_last_of("./\\");
    if (pos == StringType::npos || pos == _pathname.size() - 1 || _pathname[pos] != '.') {
        return Path();
    }
    return Path(_pathname.substr(pos));
}

Path Path::Basename() const {
    if (IsEmpty() || IsFilenameDot() || IsFilenameDotDot()) {
        return Path();
    }
    auto fileStart = _pathname.find_last_of("/\\");
    if (fileStart == StringType::npos) {
        fileStart = 0;
    }
    auto dotStart = _pathname.find_last_of(".");
    if (dotStart == StringType::npos || dotStart < fileStart) {
        return Path(_pathname.substr(fileStart + 1));
    }
    return Path(_pathname.substr(fileStart + 1, dotStart - fileStart - 1));
}

Path Path::Filename() const {
    if (IsEmpty()) {
        return Path();
    }

    auto pos = _pathname.find_last_of("/\\");
    if (pos == StringType::npos) {
        // relative path, return the whole path as filename.
        return *this;
    }
    return Path(_pathname.substr(pos + 1));
}

bool Path::HasUncPrefix() const {
    return ParseUncPrefix(_pathname);
}

bool Path::IsAbsolute() const {
    if (IsEmpty()) {
        return false;
    }

    StringType::size_type pathStart = 0;
    if (HasUncPrefix()) {
        pathStart += 4;
    }

    bool hasDriveSpec = HasDriveSpec();
    if (hasDriveSpec) {
        pathStart += 2;
    }

    if (pathStart >= _pathname.size()) {
        return false;
    }

    return IsSeparator(_pathname[pathStart]);
}

Path Path::Relative(const Path& base) const {
    Path current = Absolute().Normalize();
    Path other = base.Absolute().Normalize();

    if (current.IsEmpty() || other.IsEmpty()) {
        return Path();
    }

    std::string driveSpecCurrent;
    std::string driveSpecOther;

    std::vector<StringType> segsCurrent;
    std::vector<StringType> segsOther;

    Parse(current._pathname, nullptr, &driveSpecCurrent, nullptr, &segsCurrent);
    Parse(other._pathname, nullptr, &driveSpecOther, nullptr, &segsOther);

    // Return current path if base is from different drive spec.
    if (utils::string::ToLowerCopy(driveSpecCurrent) != utils::string::ToLowerCopy(driveSpecOther)) {
        return current;
    }

    StringType::size_type same = 0;
    for (; same < segsCurrent.size() && same < segsOther.size(); ++same) {
        if (utils::string::ToLowerCopy(segsCurrent[same]) != utils::string::ToLowerCopy(segsOther[same])) {
            break;
        }
    }

    std::stringstream ss;
    for (size_t i = 0; i < segsOther.size() - same; ++i) {
        ss << ".." << platform::DIR_SEPARATOR;
    }

    for (size_t i = same; i < segsCurrent.size(); ++i) {
        ss << segsCurrent[i];
        if (i != segsCurrent.size() - 1) {
            ss << platform::DIR_SEPARATOR;
        }
    }
    return Path(ss.str()).RemoveTrailingSeparator();
}

Path Path::Absolute() const {
    if (IsEmpty()) {
        return Path();
    }
    return CurrentDirectory() / *this;
}

Path& Path::RemoveTrailingSeparator() {
    if (_pathname.size() > 0 && IsSeparator(_pathname[_pathname.size() - 1])) {
        _pathname.erase(_pathname.size() - 1);
    }
    return *this;
}

std::ostream& operator<<(std::ostream& stream, const Path& path) {
    stream << path.String();
    return stream;
}

bool operator==(const Path& lhs, const Path& rhs) {
    return lhs.Compare(rhs) == 0;
}

bool operator!=(const Path& lhs, const Path& rhs) {
    return lhs.Compare(rhs) != 0;
}

bool operator<(const Path& lhs, const Path& rhs) {
    return lhs.Compare(rhs) < 0;
}

Path CurrentDirectory() {
    static constexpr size_t DEFAULT_PATH_SIZE = 1024;
    char path[DEFAULT_PATH_SIZE];

#ifdef SUPPORT_POSIX
    auto result = getcwd(path, sizeof(path));
    if (result == nullptr) {
        return Path();
    }
#else
    DWORD result = ::GetCurrentDirectoryA(sizeof(path), path);
    if (result == 0) {
        return Path();
    }
#endif
    return Path(path);
}

bool SetCurrentDirectory(const Path& path) {
#ifdef SUPPORT_POSIX
    return ::chdir(path.c_str()) == 0;
#else
    return ::SetCurrentDirectoryA(path.c_str()) == TRUE;
#endif
}

Path ProgramPath() {
    static constexpr size_t DEFAULT_PATH_SIZE = 1024;
    char path[DEFAULT_PATH_SIZE] = { '\0' };

#ifdef SUPPORT_POSIX
    if (::readlink("/proc/self/exe", path, sizeof(path)) > 0
        || ::readlink("/proc/curproc/file", path, sizeof(path)) > 0
        || ::readlink("/proc/self/path/a.out", path, sizeof(path))) {
        return path;
    }
#else
    if (::GetModuleFileNameA(NULL, path, sizeof(path)) > 0) {
        return path;
    }
#endif
    return Path();
}

bool Exists(const Path& path) {
#ifdef SUPPORT_POSIX
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
#else
    auto attribute = GetFileAttributesA(path.c_str());
    return attribute != INVALID_FILE_ATTRIBUTES;
#endif
}

bool IsRegularFile(const Path& path) {
#ifdef SUPPORT_POSIX
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
#else
    // We don't deal with symlink for now.
    auto attribute = GetFileAttributesA(path.c_str());
    return attribute != INVALID_FILE_ATTRIBUTES && !(attribute & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

bool IsDirectory(const Path& path) {
#ifdef SUPPORT_POSIX
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#else
    // We don't deal with symlink for now.
    auto attribute = GetFileAttributesA(path.c_str());
    return attribute != INVALID_FILE_ATTRIBUTES && attribute & FILE_ATTRIBUTE_DIRECTORY;
#endif
}

bool MakeDirectory(const Path& path) {
#ifdef SUPPORT_POSIX
    return ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0 || errno == EEXIST;
#else
    return ::CreateDirectoryA(path.c_str(), 0) == TRUE || ::GetLastError() == ERROR_ALREADY_EXISTS;
#endif
}

bool MakeDirectories(const Path& path) {
    if (path.IsEmpty()) {
        return false;
    }
    if (IsDirectory(path)) {
        return true;
    }
    auto parent = path.Parent().Normalize();
    if (!IsDirectory(parent) && !MakeDirectories(parent)) {
        return false;
    }
    return MakeDirectory(path);
}

PathIterator::PathIterator(Path path)
    : _base(std::move(path)) {
#ifdef SUPPORT_POSIX
    _dir = ::opendir(_base.c_str());
#else
    _first = true;
    WIN32_FIND_DATAA findData;
    std::string pattern = _base.String() + "\\*";
    _findHandle = ::FindFirstFileA(pattern.c_str(), &findData);

    if (_findHandle != INVALID_HANDLE_VALUE) {
        _currentPath = _base / findData.cFileName;
    }
#endif
}

PathIterator::~PathIterator() {
#ifdef SUPPORT_POSIX
    if (_dir != nullptr) {
        (void)closedir(_dir);
    }
#else
    if (_findHandle != INVALID_HANDLE_VALUE) {
        (void)::FindClose(_findHandle);
    }
#endif
}

const Path& PathIterator::operator*() const {
    return _currentPath;
}

const Path* PathIterator::operator->() const {
    return &_currentPath;
}

bool PathIterator::Next() {
#ifdef SUPPORT_POSIX
    if (_dir == nullptr) {
        return false;
    }
    struct dirent *dp;
    dp = ::readdir(_dir);
    if (dp == nullptr) {
        return false;
    }
    _currentPath = _base / dp->d_name;
#else
    if (_findHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (_first) {
        _first = false;
    }
    else {
        WIN32_FIND_DATAA findData;
        if (::FindNextFileA(_findHandle, &findData) == FALSE) {
            return false;
        }
        _currentPath = _base / findData.cFileName;
    }
#endif

    if (_currentPath.IsFilenameDot() || _currentPath.IsFilenameDotDot()) {
        // Don't include '.' or '..' as files in a directory.
        return Next();
    }

    return true;
}

}
}
