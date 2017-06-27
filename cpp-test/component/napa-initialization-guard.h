#pragma once

#include "providers/providers.h"
#include "settings/settings.h"
#include "v8/v8-common.h"

class NapaInitializationGuard {
public:
    NapaInitializationGuard() {
        static NapaInitialization initialization;
    }

private:
    class NapaInitialization {
    public:
        NapaInitialization() {
            napa::PlatformSettings settings;
            settings.loggingProvider = "console";
            napa::providers::Initialize(settings);

            napa::v8_common::Initialize();
        }

        ~NapaInitialization() {
            napa::v8_common::Shutdown();
        }
    };
};
