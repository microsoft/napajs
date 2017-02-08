#pragma once

#include "napa-c.h"

#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace napa {

    /// <summary> Response class. </summary>
    struct Response {
        Response() : code(NAPA_RESPONSE_UNDEFINED) {}

        Response(napa_container_response response) :
            code(response.code),
            errorMessage(NAPA_STRING_REF_TO_STD_STRING(response.error_message)),
            returnValue(NAPA_STRING_REF_TO_STD_STRING(response.return_value)) {}

        /// <summary> Response code. </summary>
        NapaResponseCode code;

        /// <summary> Error message. Empty when response code is success. </summary>
        std::string errorMessage;

        /// <summary> Napa return value. </summary>
        std::string returnValue;
    };

    /// <summary> Response callback signature. </summary>
    typedef std::function<void(Response)> RunCallback;
    typedef std::function<void(NapaResponseCode)> LoadCallback;

    /// <summary> C++ class wrapper around napa C APIs. </summary>
    class Container {
    public:
        
        /// <summary> Creates a container instance. </summary>
        Container();

        /// <summary> Creates a container instance. </summary>
        /// <param name="settings"> A settings string to set conainer specific settings. </param>
        Container(const std::string& settings);

        /// <summary> Deletes the underlying container handle, hence cleaning all container resources. </summary>
        ~Container();

        /// <summary> Sets a value in container scope. </summary>
        /// <param name="key"> A unique identifier for the value. </param>
        /// <param name="value"> The value. </param>
        NapaResponseCode SetGlobalValue(const std::string& key, void* value);

        /// <summary> Loads a JS file into the container asynchronously. </summary>
        /// <param name="file"> The JS file. </param>
        /// <param name="callback"> A callback that is triggered when loading is done. </param>
        void LoadFile(const std::string& file, LoadCallback callback);

        /// <summary> Loads a JS file into the container synchronously. </summary>
        /// <param name="file"> The JS file. </param>
        NapaResponseCode LoadFileSync(const std::string& file);

        /// <summary> Loads a JS source into the container asynchronously. </summary>
        /// <param name="source"> The JS source. </param>
        /// <param name="callback"> A callback that is triggered when loading is done. </param>
        void Load(const std::string& source, LoadCallback callback);

        /// <summary> Loads a JS source into the container synchronously. </summary>
        /// <param name="source"> The JS source. </param>
        NapaResponseCode LoadSync(const std::string& source);

        /// <summary> Runs a pre-loaded JS function asynchronously. </summary>
        /// <param name="func"> The name of the function to run. </param>
        /// <param name="args"> The arguments to the function. </param>
        /// <param name="callback"> A callback that is triggered when execution is done. </param>
        /// <param name="timeout"> Timeout in milliseconds - default is inifinite. </param>
        void Run(const char* func, const std::vector<NapaStringRef>& args, RunCallback callback, uint32_t timeout = 0);

        /// <summary> Runs a pre-loaded JS function synchronously. </summary>
        /// <param name="func"> The name of the function to run. </param>
        /// <param name="args"> The arguments to the function. </param>
        /// <param name="timeout"> Timeout in milliseconds - default is inifinite. </param>
        Response RunSync(const char* func, const std::vector<NapaStringRef>& args, uint32_t timeout = 0);

    private:
        napa_container_handle _handle;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Implementation starts here.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// <summary> Initialize napa with global scope settings. </summary>
    inline NapaResponseCode Initialize(const std::string& settings) {
        return napa_initialize(STD_STRING_TO_NAPA_STRING_REF(settings));
    }

    /// <summary> Initialize napa using console provided arguments. </summary>
    inline NapaResponseCode InitializeFromConsole(int argc, char* argv[]) {
        return napa_initialize_from_console(argc, argv);
    }

    /// <summary> Shut down napa. </summary>
    inline NapaResponseCode Shutdown() {
        return napa_shutdown();
    }

    /// <summary> Helper classes and functions in internal namespace. </summary>
    namespace internal {
        template <typename CallbackType>
        struct AsyncCompletionContext {
            AsyncCompletionContext(CallbackType&& callback) : callback(std::forward<CallbackType>(callback)) {}

            /// <summary> Non copyable and non movable. </summary>
            AsyncCompletionContext(const AsyncCompletionContext&) = delete;
            AsyncCompletionContext& operator=(const AsyncCompletionContext&) = delete;
            AsyncCompletionContext(AsyncCompletionContext&&) = delete;
            AsyncCompletionContext& operator=(AsyncCompletionContext&&) = delete;

            /// <summary> User callback. </summary>
            CallbackType callback;
        };

        template <typename CallbackType, typename ResponseType>
        inline void CompletionHandler(ResponseType response, void* context) {
            std::unique_ptr<AsyncCompletionContext<CallbackType>> completionContext(
                reinterpret_cast<AsyncCompletionContext<CallbackType>*>(context));

            completionContext->callback(response);
        }
    }

    inline Container::Container() : Container("") {}

    inline Container::Container(const std::string& settings) {
        _handle = napa_container_create();

        napa_container_init(_handle, STD_STRING_TO_NAPA_STRING_REF(settings));
    }

    inline Container::~Container() {
        napa_container_release(_handle);
    }

    inline NapaResponseCode Container::SetGlobalValue(const std::string& key, void* value) {
        return napa_container_set_global_value(_handle, STD_STRING_TO_NAPA_STRING_REF(key), value);
    }

    inline void Container::LoadFile(const std::string& file, LoadCallback callback) {
        auto context = new internal::AsyncCompletionContext<LoadCallback>(std::move(callback));

        napa_container_load_file(
            _handle,
            STD_STRING_TO_NAPA_STRING_REF(file),
            internal::CompletionHandler<LoadCallback, NapaResponseCode>,
            context);
    }

    inline NapaResponseCode Container::LoadFileSync(const std::string& file) {
        return napa_container_load_file_sync(_handle, STD_STRING_TO_NAPA_STRING_REF(file));
    }

    inline void Container::Load(const std::string& source, LoadCallback callback) {
        auto context = new internal::AsyncCompletionContext<LoadCallback>(std::move(callback));

        napa_container_load(
            _handle,
            STD_STRING_TO_NAPA_STRING_REF(source),
            internal::CompletionHandler<LoadCallback, napa_response_code>,
            context);
    }

    inline NapaResponseCode Container::LoadSync(const std::string& source) {
        return napa_container_load_sync(_handle, STD_STRING_TO_NAPA_STRING_REF(source));
    }

    inline void Container::Run(const char* func,
                               const std::vector<NapaStringRef>& args,
                               RunCallback callback,
                               uint32_t timeout) {
        auto context = new internal::AsyncCompletionContext<RunCallback>(std::move(callback));

        napa_container_run(
            _handle,
            CREATE_NAPA_STRING_REF(func),
            args.size(),
            args.data(),
            internal::CompletionHandler<RunCallback, napa_container_response>,
            context,
            timeout);
    }

    inline Response Container::RunSync(const char* func,
                                       const std::vector<NapaStringRef>& args,
                                       uint32_t timeout) {
        napa_container_response response = napa_container_run_sync(
            _handle,
            CREATE_NAPA_STRING_REF(func),
            args.size(),
            args.data(),
            timeout);

        return Response(response);
    }
}