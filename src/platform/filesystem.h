#pragma once

#include "utils/string.h"

#include <algorithm>
#include <cstdio>
#include <deque>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32

#include <windows.h>

#else

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif

namespace napa {
namespace filesystem {

    /// <summary> Helper class for path manipulation. </summary>
    class Path {
    public:

        // We only support ANSI path for now.
        // TODO: consider use wchar_t for Windows.
        typedef char CharType;

        typedef std::basic_string<CharType> StringType;

#ifdef _WIN32
        static constexpr char PREFERED_SEPARATOR = '\\';
#else
        static constexpr char PREFERED_SEPARATOR = '/';
#endif

        Path() {}

        Path(const CharType* path) : _pathname(path) {}
        Path(const StringType& path) : _pathname(path) {}
        Path(StringType&& path) : _pathname(std::move(path)) {}

        Path(const Path& path) : _pathname(path._pathname) {}
        Path(Path&& path) : _pathname(std::move(path._pathname)) {}

        Path& operator=(const CharType* path) {
            _pathname = path;
            return *this;
        }

        Path& operator=(const StringType& path) {
            _pathname = path;
            return *this;
        }

        Path& operator=(const Path& path) {
            _pathname = path._pathname;
            return *this;
        }

        Path& operator=(StringType&& path) {
            _pathname = std::move(path);
            return *this;
        }

        Path& operator=(Path&& path) {
            _pathname = std::move(path._pathname);
            return *this;
        }

        bool operator==(const Path& path) const {
            return _pathname == path._pathname;
        }

        bool operator!=(const Path& path) const {
            return !(this->operator==(path));
        }

        bool operator<(const Path& path) const {
            return _pathname < path._pathname;
        }

        /// <summary> Operator to append a relative path. </summmary>
        Path& operator/=(const Path& path) {
            return Append(path);
        }

        /// <summary> Operator to append a relative path. </summmary>
        Path operator/(const Path& path) const {
            Path tmp(*this);
            tmp /= path;
            return tmp;
        }

        /// <summary> Get unnormalized path string. </summary>
        /// <remarks> Use GenericForm() to always use '/' as delimiter. </summary>
        const std::string& String() const {
            return _pathname;
        }

        /// <summary> Get unnormalized path string buffer. </summary>
        const CharType* c_str() const {
            return _pathname.c_str();
        }

        /// <summary> 
        ///		Append a path to current path
        ///		Or return the path if it's absolute.
        /// </summary>
        Path& Append(const Path& path) {
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
                        _pathname += PREFERED_SEPARATOR;
                    }
                    _pathname += path._pathname;
                }
            }
            return *this;
        }

        /// <summary> Normalize current path. </summary>
        Path& Normalize() {
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
                    _pathname = FormatPathString(hasUncPrefix, driveSpec, isAbsolute, segs, PREFERED_SEPARATOR);
                }
            }
            return *this;
        }

        /// <summary> Replace extension. </summary>
        Path& ReplaceExtension(const std::string& extension) {
            auto pos = _pathname.find_last_of("./\\");
            if (pos != StringType::npos && pos != _pathname.size() - 1 && _pathname[pos] == '.') {
                _pathname.replace(pos, _pathname.size(), extension);
            }
            return *this;
        }

        /// <summary> Get normalized generic form of path string. using '/' as separator. </summary>
        Path GenericForm() const {
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

        /// <summary> Drive spec. </summary>
        Path DriveSpec() const {
            return Path(ParseDriveSpec(_pathname));
        }

        /// <summary> Get parent path. (normalized) </summary>
        Path Parent() const {
            if (IsEmpty()) {
                return Path();
            }
            return *this / "..";
        }

        /// <summary> Return directory name. </summary>
        Path Dirname() const {
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

        /// <summary> Return file extension of current path. </summary>
        Path Extension() const {
            auto pos = _pathname.find_last_of("./\\");
            if (pos == StringType::npos || pos == _pathname.size() - 1 || _pathname[pos] != '.') {
                return Path();
            }
            return Path(_pathname.substr(pos));
        }

        /// <summary> Return base file name without extension. </summary>
        Path Basename() const {
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

        /// <summary> Return file name of current path. </summary>
        Path Filename() const {
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

        /// <summary> Has UNC prefix \\? (Windows). </summary>
        bool HasUncPrefix() const {
            return ParseUncPrefix(_pathname);
        }

        /// <summary> Has drive specification like c: (Windows). </summary>
        bool HasDriveSpec() const {
            return !DriveSpec().IsEmpty();
        }

        /// <summary> Tell if path has file name. </summary>
        bool HasFilename() const {
            return !Filename().IsEmpty();
        }

        /// <summary> Tell if path has extension. </summary>
        bool HasExtension() const {
            return !Extension().IsEmpty();
        }

        /// <summary> Tell if current path is empty. </summary>
        bool IsEmpty() const {
            return _pathname.empty();
        }

        /// <summary> Tell if current path is an absolute path. </summary>
        bool IsAbsolute() const {
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

        /// <summary> Tell if current path is relative or not. </summary>
        bool IsRelative() const {
            return !IsAbsolute();
        }

        /// <summary> Tell if current file name is '.'. </summary>
        bool IsFilenameDot() const {
            return Filename().String() == ".";
        }

        /// <summary> Tell if current file name is '..'. </summary>
        bool IsFilenameDotDot() const {
            return Filename().String() == "..";
        }

        /// <summary> Get relative path to a base. </summary>
        Path Relative(const Path& base) const {
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
                ss << ".." << PREFERED_SEPARATOR;
            }

            for (size_t i = same; i < segsCurrent.size(); ++i) {
                ss << segsCurrent[i];
                if (i != segsCurrent.size() - 1) {
                    ss << PREFERED_SEPARATOR;
                }
            }
            return Path(ss.str()).RemoveTrailingSeparator();
        }

        /// <summary> Get normalized absolute path. </summary>
        Path Absolute() const;

    private:
        /// <summary> Remove one trailing separator. </summary>
        Path& RemoveTrailingSeparator() {
            if (_pathname.size() > 0 && IsSeparator(_pathname[_pathname.size() - 1])) {
                _pathname.erase(_pathname.size() - 1);
            }
            return *this;
        }

        /// <summary> Tell if a character is path separator. </summary>
        static bool IsSeparator(char ch) {
#ifdef _WIN32
            return ch == '/' || ch == '\\';
#else
            return ch == '/';
#endif
        }

        /// <summary> Tell if a character is a letter. </summary>
        static bool IsLetter(char ch) {
            return (ch >= 'A' && ch <= 'Z')
                || (ch >= 'a' && ch <= 'z');
        }

        /// <summary> Tell if it's a root of Windows path. </summary>
        static bool IsWindowsRootSegment(const StringType& segment) {
            return segment.size() == 2 && IsLetter(segment[0]) && segment[1] == ':';
        }

        /// <summary> Has UNC prefix "\\\\?" in windows. </summary>
        static bool ParseUncPrefix(const std::string& path) {
            if (path.size() < 4) {
                return false;
            }
            return path[0] == '\\' 
                && path[1] == '\\' 
                && path[2] == '?'
                && path[3] == '\\';
        }

        /// <summary> Parse driver spec. </summary>
        static std::string ParseDriveSpec(const std::string& path, bool* hasUncPrefix = nullptr) {
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
        static void Parse(
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
        static std::string FormatPathString(
            bool hasUncPrefix,
            const std::string& driveSpec,
            bool isAbsolute,
            const std::vector<StringType>& segments,
            CharType seperator = PREFERED_SEPARATOR) {

            // Output normalized string.
            std::stringstream ss;

            if (hasUncPrefix) {
                ss << "\\\\?\\";
            }

            // Drive spec could be empty.
            ss << driveSpec;

            if (isAbsolute) {
                ss << PREFERED_SEPARATOR;
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

        /// <summary> Unnormalized path. </summary>
        StringType _pathname;
    };

    /// <summary> Support outputing to ostream. </summary>
    inline std::ostream& operator<<(std::ostream& stream, const Path& path) {
        stream << path.String();
        return stream;
    }

    /// <summary> Get current working directory. </summary>
    static inline Path CurrentDirectory() {
        static constexpr size_t DEFAULT_PATH_SIZE = 1024;
        char path[DEFAULT_PATH_SIZE];

#ifdef _WIN32
        DWORD result = ::GetCurrentDirectoryA(sizeof(path), path);
        if (result == 0) {
            return Path();
        }
#else
        auto result = getcwd(path, sizeof(path));
        if (result == nullptr) {
            return Path();
        }
#endif
        return Path(path);
    }

    /// <summary> Set current working directory. </summary>
    static inline bool SetCurrentDirectory(const Path& path) {
#ifdef _WIN32
        return ::SetCurrentDirectoryA(path.c_str()) == TRUE;
#else
        return ::chdir(path.c_str()) == 0;
#endif
    }

    /// <summary> Delay implementation of Path::Absolute after CurrentDirectory. </summary>
    inline Path Path::Absolute() const {
        if (IsEmpty()) {
            return Path();
        }
        return CurrentDirectory() / *this;
    }

    /// <summary> Get path of current process. </summary>
    static inline Path ProgramPath() {
        static constexpr size_t DEFAULT_PATH_SIZE = 1024;
        char path[DEFAULT_PATH_SIZE] = { '\0' };

#ifdef _WIN32
        if (::GetModuleFileNameA(NULL, path, sizeof(path)) > 0) {
            return path;
        }
#else
        if (::readlink("/proc/self/exe", path, sizeof(path)) > 0
            || ::readlink("/proc/curproc/file", path, sizeof(path>)) > 0
            || ::readlink("/proc/self/path/a.out", path, sizeof(path))) {
            return path;
        }
#endif
        return Path();
    }

    /// <summary> Tell if a path exists (either a regular file or directory). </summary>
    inline bool Exists(const Path& path) {
#ifdef _WIN32
        auto attribute = GetFileAttributesA(path.c_str());
        return attribute != INVALID_FILE_ATTRIBUTES;
#else
        struct stat st;
        return ::stat(path.c_str(), &st) == 0;
#endif
    }

    /// <summary> Tell if a path is a regular file. </summary>
    inline bool IsRegularFile(const Path& path) {
#ifdef _WIN32
        // We don't deal with symlink for now.
        auto attribute = GetFileAttributesA(path.c_str());
        return attribute != INVALID_FILE_ATTRIBUTES && !(attribute & FILE_ATTRIBUTE_DIRECTORY);
#else
        struct stat st;
        return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
#endif
    }

    /// <summary> Tell if a path is a directory. </summary>
    inline bool IsDirectory(const Path& path) {
#ifdef _WIN32
        // We don't deal with symlink for now.
        auto attribute = GetFileAttributesA(path.c_str());
        return attribute != INVALID_FILE_ATTRIBUTES && attribute & FILE_ATTRIBUTE_DIRECTORY;
#else
        struct stat st;
        return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#endif
    }

    /// <summary> Make a directory. </summary>
    /// <returns> True if succeed or directory already exists, false if operation failed. </returns>
    inline bool MakeDirectory(const Path& path) {
#ifdef _WIN32
        return ::CreateDirectoryA(path.c_str(), 0) == TRUE || ::GetLastError() == ERROR_ALREADY_EXISTS;
#else
        return ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0 || ::errno() == EEXIST;
#endif
    }

    /// <summary> Make directories recursively. </summary>
    inline bool MakeDirectories(const Path& path) {
        if (IsDirectory(path)) {
            return true;
        }
        auto parent = path.Parent();
        if (!IsDirectory(parent) && !MakeDirectories(parent)) {
            return false;
        }
        return MakeDirectory(path);
    }

    /// <summary> Path iterator </summary>
    class PathIterator {
    public:
        PathIterator(Path path)
            : _base(std::move(path)) {
#ifdef _WIN32
            _first = true;
            WIN32_FIND_DATAA findData;
            std::string pattern = _base.String() + "\\*";
            _findHandle = ::FindFirstFileA(pattern.c_str(), &findData);

            if (_findHandle != INVALID_HANDLE_VALUE) {
                _currentPath = _base / findData.cFileName;
            }
#else
            _dir = ::opendir(_base.c_str());
#endif
        }

        ~PathIterator() {
#ifdef _WIN32
            if (_findHandle != INVALID_HANDLE_VALUE) {
                (void)::FindClose(_findHandle);
            }
#else
            if (_dir != nullptr) {
                (void)closedir(_dir);
            }
#endif
        }

        /// <summary> Operator to get the path reference. </summary>
        const Path& operator*() const {
            return _currentPath;
        }

        /// <summary> Operator to get the path pointer. </summary>
        const Path* operator->() const {
            return &_currentPath;
        }

        /// <summary> Move to next file, or return false. </summary>
        /// <remarks> Always call Next() after construction. </remarks>
        bool Next() {
#ifdef _WIN32
            if (_first) {
                _first = false;
                return INVALID_HANDLE_VALUE != _findHandle;
            }

            WIN32_FIND_DATAA findData;
            if (::FindNextFileA(_findHandle, &findData) == FALSE) {
                return false;
            }
            _currentPath = _base / findData.cFileName;
            return true;
#else
            struct dirent *dp;
            dp = ::readdir(_dir);
            if (dp == nullptr) {
                return false;
            }
            _currentPath = _base / dp->d_name;
            return true;
#endif
        }

    private:
#ifdef _WIN32
        HANDLE _findHandle;
        bool _first;
#else
        DIR* _dir;
#endif
        Path _base;
        Path _currentPath;
    };
}
}