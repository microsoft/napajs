#include <catch.hpp>
#include <napa/object-transport-v8-helpers.h>

using namespace napa::module;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:

    virtual void* Allocate(size_t length) override {
        void* data = AllocateUninitialized(length);
        return memset(data, 0, length);
    }

    virtual void* AllocateUninitialized(size_t length) override {
        return malloc(length);
    }

    virtual void Free(void* data, size_t length) override {
        free(data);
    }
};

TEST_CASE("v8-helpers works as expected", "[v8-helpers]") {
    ArrayBufferAllocator allocator;
    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = &allocator;
    auto isolate = v8::Isolate::New(createParams);

    {
        v8::Locker locker(isolate);

        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);

        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope contextScope(context);

        SECTION("pointer value is serialized/deserialized correctly", "[v8-helpers]") {
            auto source = reinterpret_cast<uintptr_t>(isolate);
            auto v8array = object_transport::UintptrToV8Uint32Array(isolate, source);
            auto target = object_transport::V8Uint32ArrayToUintptr(isolate, v8array);
            REQUIRE(target.second == true);
            REQUIRE(target.first == source);
        }

        SECTION("pointer is serialized/deserialized correctly", "[v8-helpers]") {
            auto v8array = object_transport::PtrToV8Uint32Array(isolate, isolate);
            auto target = object_transport::V8Uint32ArrayToPtr(isolate, v8array);
            REQUIRE(target.second == true);
            REQUIRE(target.first == isolate);
        }
    }

    isolate->Dispose();
}