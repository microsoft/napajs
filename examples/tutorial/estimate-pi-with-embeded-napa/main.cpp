#include <napa.h>

#include <iostream>
#include <fstream>
#include <sstream>

void ReadJSFromFile(std::stringstream& jsStream, const std::string& jsFileName);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "usage: napa-runner <js-file>" << std::endl;
        exit(0);
    }

    // Read js from user specifed js file.
    std::stringstream jsStream;
    ReadJSFromFile(jsStream, argv[1]);

    // Initializes napa with glabal scope settings.
    napa::Initialize();

    // Create a napa zone with 1 worker.
    auto zone = std::make_unique<napa::Zone>("z1", "--workers\t1");

    // Broadcast js workload.
    zone->BroadcastSync(jsStream.str());

    // Do some work in the host service.
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

void ReadJSFromFile(std::stringstream& jsStream, const std::string& jsFileName) {
    std::ifstream jsFile;
    jsFile.open(jsFileName);
    if (!jsFile) {
        std::cout << "Failed to open js file: " << jsFileName << std::endl;
        exit(0);
    }
    jsStream << jsFile.rdbuf();
    jsFile.close();
}

