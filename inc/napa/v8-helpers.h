#ifndef NAPA_V8_HELPERS_H
#define NAPA_V8_HELPERS_H

#include <sstream>
#include <unordered_map>



#define CHECK_ARG_COMMON(isolate, expression, message, result, function, line)                                \
if (!(expression))                                                                                            \
{                                                                                                             \
    std::stringstream temp;                                                                                   \
    temp << function << ":" << line << " -- " << message;                                                     \
    isolate->ThrowException(v8::Exception::TypeError(                                                         \
        v8::String::NewFromUtf8(isolate, temp.str().c_str(), v8::NewStringType::kNormal).ToLocalChecked()));  \
    return result;                                                                                            \
}

#define CHECK_ARG(isolate, expression, message)                                         \
    CHECK_ARG_COMMON(isolate, expression, message, /* empty */, __FUNCTION__, __LINE__)

#define CHECK_ARG_WITH_RETURN(isolate, expression, message, result)                     \
    CHECK_ARG_COMMON(isolate, expression, message, result, __FUNCTION__, __LINE__)

namespace napa
{
namespace v8_helpers
{

    template <typename T>
    inline T To(const v8::Local<v8::Value>& value)
    {
        static_assert(sizeof(T) == -1, "No specilization exists for this type");
    }

    template <>
    inline std::string To(const v8::Local<v8::Value>& value)
    {
        v8::String::Utf8Value utf8Value(value);
        return *utf8Value;
    }

    template <typename ValueType>
    inline std::unordered_map<std::string, ValueType> V8ObjectToMap(
        v8::Isolate* isolate,
        const v8::Local<v8::Object>& obj)
    {
        auto context = isolate->GetCurrentContext();

        std::unordered_map<std::string, ValueType> res;

        auto maybeProps = obj->GetOwnPropertyNames(context);
        if (!maybeProps.IsEmpty())
        {
            auto props = maybeProps.ToLocalChecked();
            res.reserve(props->Length());

            for (uint32_t i = 0; i < props->Length(); i++)
            {
                auto key = props->Get(context, i).ToLocalChecked();
                auto value = obj->Get(context, key).ToLocalChecked();

                v8::String::Utf8Value keyString(key->ToString());

                res.emplace(*keyString, To<ValueType>(value));
            }
        }

        return res;
    }

    template <typename ValueType>
    inline std::vector<ValueType> V8ArrayToVector(
        v8::Isolate* isolate,
        const v8::Local<v8::Array>& array)
    {
        auto context = isolate->GetCurrentContext();

        std::vector<ValueType> res;
        res.reserve(array->Length());

        for (uint32_t i = 0; i < array->Length(); i++)
        {
            res.emplace_back(To<ValueType>(array->Get(context, i).ToLocalChecked()));
        }

        return res;
    }
}
}


#endif
