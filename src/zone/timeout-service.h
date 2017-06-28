#pragma once

#include "task.h"

#include <chrono>
#include <functional>
#include <memory>

namespace napa {
namespace zone {

    /// <summary> Enables registering callbacks that will be triggered after some defined time. </summary>
    class TimeoutService {
    public:

        /// <summary>
        ///     An identifier for a single callback registration. 
        ///     If the token is destroyed before the callback is called, it will cancel the callback.
        /// </summary>
        class Token {
        public:

            /// <summary> Cancels the callback if it wasn't already triggered. </summary>
            virtual void Cancel() = 0;
            
            /// <summary> Virtual destructor. </summary>
            virtual ~Token() {}
        };

        /// <summary> Returns the single instance of this class. </summary>
        static TimeoutService& Instance();

        /// <summary> Register a callback to be called after some timeout. </summary>
        /// <param name="timeout"> The amount of time before the callback is called. </param>
        /// <param name="callback"> The callback to call. </param>
        /// <returns> 
        ///     A token that identifies the registration.
        ///     The lifetime of the token must exceed the lifetime of the passed callback, otherwise the callback
        ///     will not be called.
        /// </returns>
        std::unique_ptr<Token> Register(std::chrono::milliseconds timeout, std::function<void(void)> callback);

    private:
        struct Impl;

        explicit TimeoutService(Impl& impl);
        ~TimeoutService();

        Impl& _impl;
    };
}
}