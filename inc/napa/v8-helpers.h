#pragma once

#include <memory>
#include <sstream>
#include <unordered_map>

#include <v8.h>

namespace napa {
namespace v8_helpers {

    /// <summary> Converts a V8 string object to a movable Utf8String which supports an allocator. </summary>
    template <typename Alloc>
    class Utf8StringWithAllocator {
    public:
        Utf8StringWithAllocator(const v8::Local<v8::Value>& val, const Alloc& alloc = Alloc()) :
                                _alloc(alloc),
                                _data(nullptr),
                                _length(0) {
            if (val.IsEmpty()) {
                return;
            }
            auto isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();

            v8::Local<v8::String> str;
            if (!val->ToString(context).ToLocal(&str)) {
                return;
            }
            
            _length = str->Length();
            _data = _alloc.allocate(_length + 1);
            str->WriteUtf8(_data);
        }

        ~Utf8StringWithAllocator() {
            if (_data != nullptr) {
                _alloc.deallocate(_data, _length);
            }
        }

        const char* Data() const {
            return _data;
        }

        size_t Length() const {
            return _length;
        }

        /// <summary> Non copyable. </summary>
        Utf8StringWithAllocator(const Utf8StringWithAllocator&) = delete;
        Utf8StringWithAllocator& operator=(const Utf8StringWithAllocator&) = delete;

        /// <summary> Move constructor. </summary>
        Utf8StringWithAllocator(Utf8StringWithAllocator&& rhs) :
                                _data(rhs._data),
                                _length(rhs._length),
                                _alloc(std::move(rhs._alloc)) {
            rhs._data = nullptr;
            rhs._length = 0;
        }

        /// <summary> Move assignment. </summary>
        Utf8StringWithAllocator& operator=(Utf8StringWithAllocator&& rhs) {
            _data = rhs._data;
            _length = rhs._length;
            _alloc = std::move(rhs._alloc);

            rhs._data = nullptr;
            rhs._length = 0;

            return *this;
        }

    private:
        char* _data;
        size_t _length;
        Alloc _alloc;
    };

    typedef Utf8StringWithAllocator<std::allocator<char>> Utf8String;

    template <typename T>
    inline T V8ValueTo(const v8::Local<v8::Value>& value) {
        static_assert(sizeof(T) == -1, "No specilization exists for this type");
    }

    template <>
    inline std::string V8ValueTo(const v8::Local<v8::Value>& value) {
        v8::String::Utf8Value utf8Value(value);
        return *utf8Value;
    }

    template <>
    inline Utf8String V8ValueTo(const v8::Local<v8::Value>& value) {
        return Utf8String(value);
    }

    template <typename ValueType>
    inline std::unordered_map<std::string, ValueType> V8ObjectToMap(v8::Isolate* isolate,
                                                                    const v8::Local<v8::Object>& obj) {
        auto context = isolate->GetCurrentContext();

        std::unordered_map<std::string, ValueType> res;

        auto maybeProps = obj->GetOwnPropertyNames(context);
        if (!maybeProps.IsEmpty()) {
            auto props = maybeProps.ToLocalChecked();
            res.reserve(props->Length());

            for (uint32_t i = 0; i < props->Length(); i++) {
                auto key = props->Get(context, i).ToLocalChecked();
                auto value = obj->Get(context, key).ToLocalChecked();

                v8::String::Utf8Value keyString(key->ToString(context).ToLocalChecked());

                res.emplace(*keyString, V8ValueTo<ValueType>(value));
            }
        }

        return res;
    }

    template <typename ValueType>
    inline std::vector<ValueType> V8ArrayToVector(v8::Isolate* isolate, const v8::Local<v8::Array>& array) {
        auto context = isolate->GetCurrentContext();

        std::vector<ValueType> res;
        res.reserve(array->Length());

        for (uint32_t i = 0; i < array->Length(); i++) {
            res.emplace_back(V8ValueTo<ValueType>(array->Get(context, i).ToLocalChecked()));
        }

        return res;
    }

    // TODO @asib: Use v8::JSON::Stringify when available
    inline v8::Local<v8::Value> Stringify(v8::Isolate* isolate, const v8::Local<v8::Object>& obj) {
        auto context = isolate->GetCurrentContext();
        v8::EscapableHandleScope scope(isolate);

        auto json = context->Global()->Get(context, v8::String::NewFromUtf8(isolate, "JSON"))
            .ToLocalChecked()->ToObject(context).ToLocalChecked();

        auto stringifyFunc = json->Get(context, v8::String::NewFromUtf8(isolate, "stringify"))
            .ToLocalChecked().As<v8::Function>();

        int argc = 1;
        v8::Local<v8::Value> argv[] = { obj };

        auto res = stringifyFunc->Call(context, json, argc, argv).ToLocalChecked();

        return scope.Escape(res);
    }

    inline v8::Local<v8::String> MakeV8String(v8::Isolate *isolate, const char* str, int length = -1) {
        return v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked();
    }

    inline v8::Local<v8::String> MakeV8String(v8::Isolate *isolate, const std::string& str) {
        return MakeV8String(isolate, str.c_str(), static_cast<int>(str.length()));
    }

    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const char* data, size_t length) {
        // V8 garbage collection frees ExternalOneByteStringResourceImpl.
        auto externalResource = new v8::ExternalOneByteStringResourceImpl(data, length);
        return v8::String::NewExternalOneByte(isolate, externalResource).ToLocalChecked();
    }

    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const std::string& str) {
        return MakeExternalV8String(isolate, str.data(), str.length());
    }
}
}

#define CHECK_ARG_COMMON(isolate, expression, message, result, function, line)                                \
if (!(expression)) {                                                                                          \
    std::stringstream temp;                                                                                   \
    temp << function << ":" << line << " -- " << message;                                                     \
    isolate->ThrowException(v8::Exception::TypeError(napa::v8_helpers::MakeV8String(isolate, temp.str())));   \
    return result;                                                                                            \
}

#define CHECK_ARG(isolate, expression, message)                                         \
    CHECK_ARG_COMMON(isolate, expression, message, /* empty */, __FUNCTION__, __LINE__)

#define CHECK_ARG_WITH_RETURN(isolate, expression, message, result)                     \
    CHECK_ARG_COMMON(isolate, expression, message, result, __FUNCTION__, __LINE__)

#define JS_ASSERT(isolate, expression, message) CHECK_ARG(isolate, expression, message)

#define JS_ENSURE(isolate, expression, message) CHECK_ARG(isolate, expression, message)

#define JS_ENSURE_WITH_RETURN(isolate, expression, message, result)                     \
    CHECK_ARG_WITH_RETURN(isolate, expression, message, result)