#pragma once

#include <napa.h>

class NapaInitializationGuard {
public:
    NapaInitializationGuard() {
        static NapaInitialization initialization;
    }

private:
    class NapaInitialization {
    public:
        NapaInitialization() {
            // Only call napa::Initialize once per process.
            napa::Initialize("--loggingProvider console");
        }

        ~NapaInitialization() {
            // Only call napa::Shutdown once per process.
            napa::Shutdown();
        }
    };
};
