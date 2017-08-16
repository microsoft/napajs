// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>
#include <platform/filesystem.h>

using namespace napa;

TEST_CASE("filesystem::Path", "[Path]") {

    SECTION("Empty path") {
        filesystem::Path path;
        REQUIRE(path.IsEmpty());
        REQUIRE(!path.IsAbsolute());
        REQUIRE(path.IsRelative());

        REQUIRE(!path.IsFilenameDot());
        REQUIRE(!path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == ".");
        REQUIRE(!path.HasFilename());
        REQUIRE(path.Basename().IsEmpty());
        REQUIRE(!path.HasExtension());

        REQUIRE(path.ReplaceExtension(".exe").IsEmpty());

        REQUIRE(path.Normalize().IsEmpty());
        REQUIRE(path.Absolute().IsEmpty());
        REQUIRE(path.Relative("a/b").IsEmpty());
        REQUIRE(path.Parent().IsEmpty());
        REQUIRE(path.String() == "");
        REQUIRE(path.DriveSpec().IsEmpty());
        REQUIRE(!path.HasUncPrefix());
    }

    SECTION("Explicit relative path") {
        // ../a/c
        filesystem::Path path("../a\\b/../c");
        REQUIRE(!path.IsEmpty());
        REQUIRE(!path.IsAbsolute());
        REQUIRE(path.IsRelative());

        REQUIRE(!path.IsFilenameDot());
        REQUIRE(!path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == "../a\\b/..");
        REQUIRE(path.Filename() == "c");
        REQUIRE(path.Basename() == "c");
        REQUIRE(path.Extension().IsEmpty());

        REQUIRE(path.ReplaceExtension(".exe").Extension().IsEmpty());

#ifdef _WIN32
        REQUIRE(path.Normalize() == "..\\a\\c");
        REQUIRE(path.Parent().Normalize() == "..\\a");
        REQUIRE(path.Parent().Parent().Normalize() == "..");
        REQUIRE(path.Parent().Parent().Parent().Normalize() == "..\\..");
        REQUIRE(path.String() == "..\\a\\c");
#else
        REQUIRE(path.Normalize() == "../a/c");
        REQUIRE(path.Parent().Normalize() == "../a");
        REQUIRE(path.Parent().Parent().Normalize() == "..");
        REQUIRE(path.Parent().Parent().Parent().Normalize() == "../..");

        REQUIRE(path.String() == "../a/c");
#endif

        REQUIRE(path.GenericForm() == "../a/c");
        REQUIRE(path.DriveSpec().IsEmpty());
        REQUIRE(!path.HasUncPrefix());
    }

    SECTION("Implicit relative path") {
        filesystem::Path path("a/b\\c\\../d\\.\\e.txt");
        REQUIRE(!path.IsEmpty());
        REQUIRE(!path.IsAbsolute());
        REQUIRE(path.IsRelative());

        REQUIRE(!path.IsFilenameDot());
        REQUIRE(!path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == "a/b\\c\\../d\\.");
        REQUIRE(path.Filename() == "e.txt");
        REQUIRE(path.Basename() == "e");
        REQUIRE(path.Extension() == ".txt");

        REQUIRE(path.ReplaceExtension(".exe").Extension() == ".exe");

#ifdef _WIN32
        REQUIRE(path.Normalize() == "a\\b\\d\\e.exe");
        REQUIRE(path.Parent().Normalize() == "a\\b\\d");

        REQUIRE(path.Relative("a/b/./e") == "..\\d\\e.exe");
        REQUIRE(path.Relative("b/c/../e") == "..\\..\\a\\b\\d\\e.exe");
        REQUIRE(path.Relative(filesystem::CurrentDirectory() / "e" / "f") == "..\\..\\a\\b\\d\\e.exe");
        REQUIRE(path.Relative("F:\\a\\b") == path.Absolute().Normalize());
        REQUIRE(path.String() == "a\\b\\d\\e.exe");
#else
        REQUIRE(path.Normalize() == "a/b/d/e.exe");
        REQUIRE(path.Parent().Normalize() == "a/b/d");

        REQUIRE(path.Relative("a/b/./e") == "../d/e.exe");
        REQUIRE(path.Relative("b/c/../e") == "../../a/b/d/e.exe");
        REQUIRE(path.Relative(filesystem::CurrentDirectory() / "e" / "f") == "../../a/b/d/e.exe");

        REQUIRE(path.String() == "a/b/d/e.exe");
#endif

        REQUIRE(path.Parent().Parent().Parent().Normalize() == "a");
        REQUIRE(path.Parent().Parent().Parent().Parent().Normalize() == ".");
        REQUIRE(path.Parent().Parent().Parent().Parent().Parent().Normalize() == "..");

        REQUIRE(path.GenericForm() == "a/b/d/e.exe");
        REQUIRE(path.DriveSpec().IsEmpty());
        REQUIRE(!path.HasUncPrefix());
    }

    SECTION("Absolute path (posix)") {
        // /a
        filesystem::Path path("/a/./b/../c/..");
        REQUIRE(!path.IsEmpty());
        REQUIRE(path.IsAbsolute());
        REQUIRE(!path.IsRelative());

        REQUIRE(!path.IsFilenameDot());
        REQUIRE(path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == "/a/./b/../c");
        REQUIRE(path.Filename() == "..");
        REQUIRE(path.Basename() == "");
        REQUIRE(path.Extension() == "");

        REQUIRE(path.ReplaceExtension(".exe").Extension().IsEmpty());

#ifdef _WIN32
        REQUIRE(path.Normalize() == "\\a");
        REQUIRE(path.Parent().Normalize().String() == "\\");
        REQUIRE(path.String() == "\\a");
#else
        REQUIRE(path.Normalize() == "/a");
        REQUIRE(path.Parent().Normalize().String() == "/");
        REQUIRE(path.String() == "/a");
#endif

        REQUIRE(path.Parent().Parent().Normalize().IsEmpty());

        REQUIRE(path.Relative("/") == "a");
        REQUIRE(path.Relative("/a/c") == "..");
        REQUIRE(path.GenericForm() == "/a");
        REQUIRE(path.DriveSpec() == "");
        REQUIRE(!path.HasUncPrefix());
    }

#ifdef _WIN32
    SECTION("Absolute path (Windows)") {
        // C:/a/c/.
        filesystem::Path path("C:\\a\\./b\\..\\c/.");
        REQUIRE(!path.IsEmpty());
        REQUIRE(path.IsAbsolute());
        REQUIRE(!path.IsRelative());

        REQUIRE(path.IsFilenameDot());
        REQUIRE(!path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == "C:\\a\\./b\\..\\c");
        REQUIRE(path.Filename() == ".");
        REQUIRE(path.Basename() == "");
        REQUIRE(path.Extension() == "");

        REQUIRE(path.ReplaceExtension(".exe").Extension().IsEmpty());
        REQUIRE(path.Normalize() == "C:\\a\\c");

        REQUIRE(path.Parent().Normalize() == "C:\\a");
        REQUIRE(path.Parent().Parent().Normalize() == "C:\\");
        REQUIRE(path.Parent().Parent().Parent().Normalize().IsEmpty());

        REQUIRE(path.Relative("C:/") == "a\\c");
        REQUIRE(path.Relative("C:/a/c") == "");
        REQUIRE(path.Relative("C:/b/c") == "..\\..\\a\\c");
        REQUIRE(path.Relative("F:\\a\\b") == path.Absolute().Normalize());
        REQUIRE(path.String() == "C:\\a\\c");
        REQUIRE(path.GenericForm() == "C:/a/c");
        REQUIRE(path.DriveSpec() == "C:");
        REQUIRE(!path.HasUncPrefix());
    }

    SECTION("UNC path (windows)") {
        filesystem::Path path("\\\\?\\c:\\a\\b\\..\\c");
        REQUIRE(!path.IsEmpty());
        REQUIRE(path.IsAbsolute());
        REQUIRE(!path.IsRelative());

        REQUIRE(!path.IsFilenameDot());
        REQUIRE(!path.IsFilenameDotDot());

        REQUIRE(path.Dirname() == "\\\\?\\c:\\a\\b\\..");
        REQUIRE(path.Filename() == "c");
        REQUIRE(path.Basename() == "c");
        REQUIRE(path.Extension() == "");

        REQUIRE(path.ReplaceExtension(".exe").Extension().IsEmpty());
        REQUIRE(path.Normalize() == "\\\\?\\c:\\a\\c");

        REQUIRE(path.Parent().Normalize() == "\\\\?\\c:\\a");
        REQUIRE(path.Parent().Parent().Normalize() == "\\\\?\\c:\\");
        REQUIRE(path.Parent().Parent().Parent().Normalize().IsEmpty());

        REQUIRE(path.Relative("C:/") == "a\\c");
        REQUIRE(path.Relative("C:/a/c") == "");
        REQUIRE(path.Relative("C:/b/c") == "..\\..\\a\\c");
        REQUIRE(path.Relative("F:\\a\\b") == path.Absolute().Normalize());
        REQUIRE(path.String() == "\\\\?\\c:\\a\\c");
        REQUIRE(path.GenericForm() == "\\\\?\\c:\\a\\c");
        REQUIRE(path.DriveSpec() == "c:");
        REQUIRE(path.HasUncPrefix());
    }
#endif
}

TEST_CASE("filesystem::PathIterator", "[PathIterator]") {

    SECTION("Path not exist") {
        filesystem::PathIterator it("/a/b/c");
        REQUIRE(!it.Next());
        REQUIRE(*it == "");
        REQUIRE(it->IsEmpty());
    }

    SECTION("Path exists") {
        auto exePath = filesystem::ProgramPath();
        auto exeDir = exePath.Parent().Normalize();
        filesystem::PathIterator it(exeDir);
        REQUIRE(it.Next());

        bool hasDot = false;
        bool hasDotDot = false;
        bool hasExe = false;

        do {
            REQUIRE(!it->IsEmpty());
            REQUIRE(it->Dirname().Absolute().Normalize() == exeDir);

            if (it->IsFilenameDot()) {
                hasDot = true;
            }
            if (it->IsFilenameDotDot()) {
                hasDotDot = true;
            }
            if (*it == exePath) {
                hasExe = true;
            }
        } while (it.Next());

        REQUIRE(!hasDot);
        REQUIRE(!hasDotDot);
        REQUIRE(hasExe);
    }
}

TEST_CASE("filesystem", "[Operations]") {

    SECTION("Exists") {
        REQUIRE(filesystem::Exists("."));
        REQUIRE(filesystem::Exists(filesystem::ProgramPath()));
    }

    SECTION("IsDirectory") { REQUIRE(filesystem::IsDirectory(".")); }

    SECTION("IsRegularFile") { REQUIRE(filesystem::IsRegularFile(filesystem::ProgramPath())); }

    SECTION("MakeDirectory") {
        REQUIRE(filesystem::MakeDirectory("./abc"));
        REQUIRE(filesystem::IsDirectory("./abc"));
    }

    SECTION("MakeDirectories") {
        REQUIRE(filesystem::MakeDirectories("./a/b/c"));
        REQUIRE(filesystem::IsDirectory("./a/b/c"));
    }
}