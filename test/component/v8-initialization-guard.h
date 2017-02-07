#pragma once

#include "v8/v8-common.h"

class V8InitializationGuard {
public:
    V8InitializationGuard() {
        static V8Initialization initialization;
    }

private:
    class V8Initialization {
    public:
        V8Initialization() {
            napa::v8_common::Initialize();
        }

        ~V8Initialization() {
            napa::v8_common::Shutdown();
        }
    };
};
