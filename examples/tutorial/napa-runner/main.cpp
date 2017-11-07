// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

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
    auto mainZone = std::make_unique<napa::Zone>("main", "--workers 1");

    // Broadcast js workload.
    napa::ResultCode resultCode = mainZone->BroadcastSync(jsStream.str());
    if (resultCode != NAPA_RESULT_SUCCESS) {
        std::cout << napa_result_code_to_string(resultCode) << std::endl;
        // Shut down napa.
        napa::Shutdown();
        exit(1);
    }

    // Put a dead loop here to allow all tasks to complete in napa zone before shutting down napa.
    // We will provide a graceful shutdown method to save this.
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Shut down napa.
    napa::Shutdown();

    return 0;
}

void ReadJSFromFile(std::stringstream& jsStream, const std::string& jsFileName)
{
    std::ifstream jsFile;
    jsFile.open(jsFileName);
    if (!jsFile) {
        std::cout << "Failed to open js file: " << jsFileName << std::endl;
        exit(0);
    }
    jsStream << jsFile.rdbuf();
    jsFile.close();
}

