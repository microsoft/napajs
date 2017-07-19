// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <platform/platform.h>
#include <platform/os.h>

#include <string>

#ifdef SUPPORT_POSIX

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#else

#pragma push_macro("NOMINMAX")
#define NOMINMAX
#include <windows.h>
#pragma pop_macro("NOMINMAX")

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

        Path() {}

        Path(const CharType* path) : _pathname(path) {}
        Path(const StringType& path) : _pathname(path) {}
        Path(StringType&& path) : _pathname(std::move(path)) {}

        Path(const Path& path) : _pathname(path._pathname) {}
        Path(Path&& path) : _pathname(std::move(path._pathname)) {}

        Path& operator=(const CharType* path);
        Path& operator=(const StringType& path);
        Path& operator=(const Path& path);
        Path& operator=(StringType&& path);
        Path& operator=(Path&& path);

        /// <summary> Operator to append a relative path. </summmary>
        Path& operator/=(const Path& path);

        /// <summary> Operator to append a relative path. </summmary>
        Path operator/(const Path& path) const;

        /// <summary> Compare two paths. </summmary>
        int Compare(const Path& path) const;

        /// <summary> Get unnormalized path string. </summary>
        /// <remarks> Use GenericForm() to always use '/' as delimiter. </summary>
        const std::string& String() const;

        /// <summary> Get unnormalized path string buffer. </summary>
        const CharType* c_str() const;

        /// <summary> 
        ///	    Append a path to current path
        ///	    Or return the path if it's absolute.
        /// </summary>
        Path& Append(const Path& path);

        /// <summary> Normalize current path. </summary>
        Path& Normalize();

        /// <summary> Replace extension. </summary>
        Path& ReplaceExtension(const std::string& extension);

        /// <summary> Get normalized generic form of path string. using '/' as separator. </summary>
        Path GenericForm() const;

        /// <summary> Drive spec. </summary>
        Path DriveSpec() const;

        /// <summary> Get parent path. (normalized) </summary>
        Path Parent() const;

        /// <summary> Return directory name. </summary>
        Path Dirname() const;

        /// <summary> Return file extension of current path. </summary>
        Path Extension() const;

        /// <summary> Return base file name without extension. </summary>
        Path Basename() const;

        /// <summary> Return file name of current path. </summary>
        Path Filename() const;

        /// <summary> Has UNC prefix \\? (Windows). </summary>
        bool HasUncPrefix() const;

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
        bool IsAbsolute() const;

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
        Path Relative(const Path& base) const;

        /// <summary> Get normalized absolute path. </summary>
        Path Absolute() const;

    private:
        /// <summary> Remove one trailing separator. </summary>
        Path& RemoveTrailingSeparator();

        /// <summary> Unnormalized path. </summary>
        StringType _pathname;
    };

    /// <summary> Support outputing to ostream. </summary>
    std::ostream& operator<<(std::ostream& stream, const Path& path);

    bool operator==(const Path& lhs, const Path& rhs);
    bool operator!=(const Path& lhs, const Path& rhs);
    bool operator<(const Path& lhs, const Path& rhs);

    /// <summary> Get current working directory. </summary>
    Path CurrentDirectory();

    /// <summary> Set current working directory. </summary>
    bool SetCurrentDirectory(const Path& path);

    /// <summary> Get path of current process. </summary>
    Path ProgramPath();

    /// <summary> Tell if a path exists (either a regular file or directory). </summary>
    bool Exists(const Path& path);

    /// <summary> Tell if a path is a regular file. </summary>
    bool IsRegularFile(const Path& path);

    /// <summary> Tell if a path is a directory. </summary>
    bool IsDirectory(const Path& path);

    /// <summary> Make a directory. </summary>
    /// <returns> True if succeed or directory already exists, false if operation failed. </returns>
    bool MakeDirectory(const Path& path);

    /// <summary> Make directories recursively. </summary>
    bool MakeDirectories(const Path& path);

    /// <summary> Path iterator </summary>
    class PathIterator {
    public:
        PathIterator(Path path);
        ~PathIterator();

        /// <summary> Operator to get the path reference. </summary>
        const Path& operator*() const;

        /// <summary> Operator to get the path pointer. </summary>
        const Path* operator->() const;

        /// <summary> Move to next file, or return false. </summary>
        /// <remarks> Always call Next() after construction. </remarks>
        bool Next();

    private:

#ifdef SUPPORT_POSIX
        DIR* _dir;
#else
        HANDLE _findHandle;
        bool _first;
#endif
        Path _base;
        Path _currentPath;
    };
}
}