#include <catch/catch.hpp>

#include <platform/platform.h>
#include <module/core-modules/node/file-system-helpers.h>

using namespace napa;
using namespace napa::module;

TEST_CASE("File system helpers reads/writes a file correctly.", "[file-system-helpers]") {
    const std::string dirname("file-system-helpers-test");
    const std::string filename(dirname + platform::DIR_SEPARATOR + "file-system-helpers-test.dat");

    file_system_helpers::MkdirSync(dirname);
    file_system_helpers::WriteFileSync(filename, dirname.data(), dirname.length());

    REQUIRE(file_system_helpers::ExistsSync(filename));

    auto content = file_system_helpers::ReadFileSync(filename);
    REQUIRE(content.compare(dirname) == 0);

    file_system_helpers::MkdirSync(dirname + platform::DIR_SEPARATOR + "1");
    file_system_helpers::MkdirSync(dirname + platform::DIR_SEPARATOR + "2");

    auto names = file_system_helpers::ReadDirectorySync(dirname);
    REQUIRE(names.size() == 3);
}