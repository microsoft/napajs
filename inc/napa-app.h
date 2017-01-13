#ifndef NAPA_APP_H
#define NAPA_APP_H

#include "napa-runtime.h"


namespace napa {
namespace app {

    /// <summary> Interface for request object. </summary>
    class Request {
    public:

        /// <summary> Serialize a request to JSON string. </summary>
        /// <returns> JSON string on successful serialization. Nullptr on failure. </returns>
        virtual NapaStringRef ToJson() const = 0;

        /// <summary> Virtual destuctor. </summary>
        virtual ~Request() {}
    };

    /// <summary> A string based request. </summary>
    class StringRequest : public Request {
    public:

        StringRequest(const char* content) : _content(content) {}
        StringRequest(const std::string& content) : _content(content) {}

        virtual NapaStringRef ToJson() const override {
            return STD_STRING_TO_NAPA_STRING_REF(_content);
        }

    private:
        std::string _content;
    };

    /// <summary> A string ref based request. </summary>
    class StringRefRequest : public Request {
    public:
        StringRefRequest(const char* data) : _content(CREATE_NAPA_STRING_REF(data)) {}
        StringRefRequest(const char* data, size_t size) : _content(CREATE_NAPA_STRING_REF_WITH_SIZE(data, size)) {}
        StringRefRequest(NapaStringRef content) : _content(content) {}

        virtual NapaStringRef ToJson() const override {
            return _content;
        }

    private:
        NapaStringRef _content;
    };

    typedef napa::runtime::Response Response;

    /// <summary> Facade class to facilitate napa app users in C++  </summary>
    class Engine {
    public:
        typedef napa::runtime::RunCallback ResponseCallback;

        Engine();
        Engine(const napa::runtime::Container& container);

        void Execute(const Request& request, ResponseCallback callback);
        Response ExecuteSync(const Request& request);

    private:
        static constexpr const char* ENTRY_FILE = "napa-app-main.js";
        static constexpr const char* ENTRY_FUNCTION = "handleRequest";

        napa::runtime::Container _container;
    };

    inline Engine::Engine() : Engine(napa::runtime::Container()) {
    }

    inline Engine::Engine(const napa::runtime::Container& container) : _container(container) {
        _container.LoadFileSync(ENTRY_FILE);
    }

    inline void Engine::Execute(const Request& request, ResponseCallback callback) {
        return _container.Run(ENTRY_FUNCTION, { request.ToJson() }, std::move(callback));
    }

    inline Response Engine::ExecuteSync(const Request& request) {
        return _container.RunSync(ENTRY_FUNCTION, { request.ToJson() });
    }
}
}

#endif // NAPA_APP_H
