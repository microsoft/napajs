#include "catch.hpp"

#include "module/core-modules/file-system-helpers.h"

using namespace napa::module;

TEST_CASE("File system helpers reads/writes a file correctly.", "[file-system-helpers]") {
    const std::string dirname("file-system-helpers-test");
    const std::string filename(dirname + "\\file-system-helpers-test.dat");

    file_system_helpers::MkdirSync(dirname);
    file_system_helpers::WriteFileSync(filename, dirname.data(), dirname.length());

    REQUIRE(file_system_helpers::ExistsSync(filename));

    auto content = file_system_helpers::ReadFileSync(filename);
    REQUIRE(content.compare(dirname) == 0);
}