// Example code for CPP embedder.

#include "napa-runtime.h"

#include <iostream>

int main(int argc, char* argv[])
{
    napa::runtime::InitializeFromConsole(argc, argv);

    std::cout << napa_response_code_to_string(NAPA_RESPONSE_SUCCESS) << std::endl;

    return 0;
}
