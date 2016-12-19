#ifndef NapaRuntime_H
#define NapaRuntime_H

#include "napa-runtime-c.h"

#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace napa
{
namespace runtime
{
    /// <summary> Initialize napa runtime with global scope settings </summary>
    /// <see cref="napa_initialize" />
    inline NapaResponseCode Initialize(const std::string& settings)
    {
        return napa_initialize(STD_STRING_TO_NAPA_STRING_REF(settings));
    }

    /// <summary> Initialize napa runtime using console provided arguments </summary>
    /// <see cref="napa_initialize_from_console" />
    inline NapaResponseCode InitializeFromConsole(int argc, char* argv[])
    {
        return napa_initialize_from_console(argc, argv);
    }

    /// <summary> Shut down napa runtime </summary>
    /// <see cref="napa_shutdown" />
    inline NapaResponseCode Shutdown()
    {
        return napa_shutdown();
    }

    /// <summary>Response</summary>
    struct Response
    {
        /// <summary>Response code.</summary>
        NapaResponseCode code;

        /// <summary>Napa output. Json format in case of success, error message otherwise</summary>
        std::string output;
    };

    /// <summary>Response callback signature</summary>
    typedef std::function<void(Response)> ResponseCallback;

    /// <summary>helper classes and functions in internal namespace</summary>
    namespace internal
    {
        struct RunCompletionContext
        {
            RunCompletionContext(ResponseCallback callback)
                : callback(std::move(callback))
            {
            }

            /// <summary>Non copyable and non movable.</summary>
            RunCompletionContext(const RunCompletionContext&) = delete;
            RunCompletionContext& operator=(const RunCompletionContext&) = delete;
            RunCompletionContext(RunCompletionContext&&) = delete;
            RunCompletionContext& operator=(RunCompletionContext&&) = delete;

            /// <summary>User callback.</summary>
            ResponseCallback callback;
        };

        inline void RunCompletionHandler(napa_container_response response, void* context)
        {
            std::unique_ptr<RunCompletionContext> completionContext(
                reinterpret_cast<RunCompletionContext*>(context));

            completionContext->callback(
                Response{ response.code, NAPA_STRING_REF_TO_STD_STRING(response.output) });
        }

        inline std::vector<napa_string_ref> ConvertToNapaRuntimeArgs(const std::vector<std::string>& args)
        {
            std::vector<napa_string_ref> res;
            res.reserve(args.size());
            for (const auto& arg : args)
            {
                res.emplace_back(STD_STRING_TO_NAPA_STRING_REF(arg));
            }

            return res;
        }
    }


    /// <summary> C++ class wrapper around napa runtime C APIs </summary>
    class Container
    {
    public:

        /// <summary> Creates a container instance </summary>
        /// <param name="settings"> A settings string to set conainer specific settings </param>
        Container(const std::string& settings);

        /// <summary> Deletes the underlying container handle, hence cleaning all container resources </summary>
        ~Container();

        /// <summary> Sets a value in container scope </summary>
        /// <param name="key"> A unique identifier for the value </param>
        /// <param name="value"> The value </param>
        /// <see cref="NapaRuntime_SetGlobalValue" />
        NapaResponseCode SetGlobalValue(const std::string& key, void* value);

        /// <summary> Loads a JS file into the container </summary>
        /// <param name="file"> The JS file </param>
        /// <see cref="NapaRuntime_LoadFile" />
        NapaResponseCode LoadFile(const std::string& file);

        /// <summary> Loads a JS source into the container </summary>
        /// <param name="source"> The JS source </param>
        /// <see cref="NapaRuntime_Load" />
        NapaResponseCode Load(const std::string& source);

        /// <summary> Runs a pre-loaded JS function asynchronously </summary>
        /// <param name="func">The name of the function to run</param>
        /// <param name="args">The arguments to the function</param>
        /// <param name="callback">A callback that is triggered when execution is done</param>
        /// <param name="timeout">Timeout in milliseconds - default is inifinite</param>
        /// <see cref="NapaRuntime_Run" />
        void Run(
            const std::string& func,
            const std::vector<std::string>& args,
            ResponseCallback callback,
            uint32_t timeout = 0);

        /// <summary> Runs a pre-loaded JS function synchronously </summary>
        /// <param name="func">The name of the function to run</param>
        /// <param name="args">The arguments to the function</param>
        /// <param name="timeout">Timeout in milliseconds - default is inifinite</param>
        Response RunSync(
            const std::string& func,
            const std::vector<std::string>& args,
            uint32_t timeout = 0);

    private:
        napa_container_handle _handle;
    };


    inline Container::Container(const std::string& settings)
    {
        _handle = napa_container_create();

        napa_container_init(_handle, STD_STRING_TO_NAPA_STRING_REF(settings));
    }

    inline Container::~Container()
    {
        napa_container_release(_handle);
    }

    inline NapaResponseCode Container::SetGlobalValue(const std::string& key, void* value)
    {
        return napa_container_set_global_value(_handle, STD_STRING_TO_NAPA_STRING_REF(key), value);
    }

    inline NapaResponseCode Container::LoadFile(const std::string& file)
    {
        return napa_container_load_file(_handle, STD_STRING_TO_NAPA_STRING_REF(file));
    }

    inline NapaResponseCode Container::Load(const std::string& source)
    {
        return napa_container_load(_handle, STD_STRING_TO_NAPA_STRING_REF(source));
    }

    inline void Container::Run(
        const std::string& func,
        const std::vector<std::string>& args,
        ResponseCallback callback,
        uint32_t timeout)
    {
        auto argv = internal::ConvertToNapaRuntimeArgs(args);

        auto context = new internal::RunCompletionContext(std::move(callback));

        napa_container_run(
            _handle,
            STD_STRING_TO_NAPA_STRING_REF(func),
            argv.size(),
            argv.data(),
            internal::RunCompletionHandler,
            context,
            timeout);
    }

    inline Response Container::RunSync(
        const std::string& func,
        const std::vector<std::string>& args,
        uint32_t timeout)
    {
        auto argv = internal::ConvertToNapaRuntimeArgs(args);

        napa_container_response response = napa_container_run_sync(
            _handle,
            STD_STRING_TO_NAPA_STRING_REF(func),
            argv.size(),
            argv.data(),
            timeout);

        return Response { response.code, NAPA_STRING_REF_TO_STD_STRING(response.output) };
    }

}
}

#endif // NapaRuntime_H
