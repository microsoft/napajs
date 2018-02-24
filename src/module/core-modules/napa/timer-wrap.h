// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <zone/timer.h>

namespace napa {
namespace module {
    
    /// <summary> It wraps napa::zone::Timer. </summary>
    /// <remarks> Reference: napajs/lib/timer.ts#Timer </remarks>
    class TimerWrap: public NAPA_OBJECTWRAP {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> It creates an instance of TimerWrap with a napa::zone::Timer pointer. </summary>
        static v8::Local<v8::Object> NewInstance(std::shared_ptr<napa::zone::Timer> timer);

        napa::zone::Timer& Get();

        static void SetImmediateCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        static void SetTimeoutIntervalCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:
        /// <summary> Default constructor. </summary>
        TimerWrap() = default;
    
        /// <summary> No copy allowed. </summary>
        TimerWrap(const TimerWrap&) = delete;
        TimerWrap& operator=(const TimerWrap&) = delete;


        /// <summary> Friend default constructor callback. </summary>
        template <typename T>
        friend void napa::module::DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);

        template <typename T>
        friend v8::MaybeLocal<v8::Object> napa::module::NewInstance(int argc, v8::Local<v8::Value> argv[]);

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "TimerWrap";

        /// <summary> Hid constructor from public access. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR();

        std::shared_ptr<napa::zone::Timer> _timer;
    };
}
}
