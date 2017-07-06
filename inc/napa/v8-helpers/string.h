#pragma once

#include <v8.h>
#include <napa/stl/string.h>
#include <cstring>

namespace napa {
namespace v8_helpers {

    /// <summary> Make a V8 string by making a copy of const char*. </summary>
    inline v8::Local<v8::String> MakeV8String(v8::Isolate *isolate, const char* str, int length = -1) {
        return v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked();
    }

    /// <summary> Make a V8 string from std::string. </summary>
    inline v8::Local<v8::String> MakeV8String(v8::Isolate *isolate, const std::string& str) {
        return MakeV8String(isolate, str.c_str(), static_cast<int>(str.length()));
    }

    /// <summary> Make a V8 string from std::string. </summary>
    inline v8::Local<v8::String> MakeV8String(v8::Isolate *isolate, const napa::stl::String& str) {
        return MakeV8String(isolate, str.c_str(), static_cast<int>(str.length()));
    }

    /// <summary> Make a V8 string from external const char*. </summary>
    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const char* data) {
        // V8 garbage collection frees ExternalOneByteStringResourceImpl.
        auto externalResource = new v8::ExternalOneByteStringResourceImpl(data, std::strlen(data));
        return v8::String::NewExternalOneByte(isolate, externalResource).ToLocalChecked();
    }

    /// <summary> Make a V8 string from external const char*. </summary>
    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const char* data, size_t length) {
        // V8 garbage collection frees ExternalOneByteStringResourceImpl.
        auto externalResource = new v8::ExternalOneByteStringResourceImpl(data, length);
        return v8::String::NewExternalOneByte(isolate, externalResource).ToLocalChecked();
    }

    /// <summary> Make a V8 string from external std::string. </sumary>
    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const std::string& str) {
        return MakeExternalV8String(isolate, str.data(), str.length());
    }

     /// <summary> Make a V8 string from external napa::stl::String. </sumary>
    inline v8::Local<v8::String> MakeExternalV8String(v8::Isolate *isolate, const napa::stl::String& str) {
        return MakeExternalV8String(isolate, str.data(), str.length());
    }

    /// <summary> Converts a V8 string object to a movable Utf8String which supports an allocator. </summary>
    template <typename Alloc>
    class Utf8StringWithAllocator {
    public:
        Utf8StringWithAllocator()
            : _data(nullptr), _length(0) {
        }

        Utf8StringWithAllocator(
            const v8::Local<v8::Value>& val, 
            const Alloc& alloc = Alloc()) 
            : _alloc(alloc),
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

    /// <summary> Utf8String in C++. </summary>
    typedef Utf8StringWithAllocator<std::allocator<char>> Utf8String;
}
}