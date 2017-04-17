#include "file-system.h"
#include "file-system-helpers.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>

using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Read file synchronoulsy. </summary>
    /// <param name="args"> It holds filename. </param>
    void ReadFileSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Write file synchronoulsy. </summary>
    /// <param name="args"> It holds filename and string to write. </param>
    void WriteFileSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Make directory synchronoulsy. </summary>
    /// <param name="args"> It holds directory to make. </param>
    void MkdirSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Check if a path exits synchronously. </summary>
    /// <param name="args"> A string argument of path. </param>
    void ExistsSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Read a directory synchronously. </summary>
    /// <param name="args"> A string argument of path. </param>
    void ReaddirSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

}   // End of anonymous namespace.

void file_system::Init(v8::Local<v8::Object> exports) {
    NAPA_SET_METHOD(exports, "readFileSync", ReadFileSyncCallback);
    NAPA_SET_METHOD(exports, "writeFileSync", WriteFileSyncCallback);
    NAPA_SET_METHOD(exports, "mkdirSync", MkdirSyncCallback);
    NAPA_SET_METHOD(exports, "existsSync", ExistsSyncCallback);
    NAPA_SET_METHOD(exports, "readdirSync", ReaddirSyncCallback);
}

namespace {

    void ReadFileSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() > 0,
            "fs.readFileSync requires at least 1 argument.");

        CHECK_ARG(isolate,
            args[0]->IsString(),
            "fs.readFileSync requires a string of file path as the 1st argument.");

        v8::String::Utf8Value filename(args[0]);

        try {
            auto content = file_system_helpers::ReadFileSync(*filename);
            args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, content));
        } catch (const std::exception& ex) {
            isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
            args.GetReturnValue().SetUndefined();
        }
    }

    void WriteFileSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() >= 2,
            "fs.writeFileSync requires 2 parameters.");

        CHECK_ARG(isolate,
            args[0]->IsString(),
            "fs.writeFileSync requires a string as the 1st parameter for file name.");

        CHECK_ARG(isolate,
            args[1]->IsString(),
            "fs.writeFileSync require a string as the 2nd parameter for data to write.");
            
        v8::String::Utf8Value fileName(args[0]);
        v8::String::Utf8Value content(args[1]);

        try {
            file_system_helpers::WriteFileSync(std::string(*fileName), *content, static_cast<size_t>(content.length()));
        } catch (const std::exception& ex) {
            isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
        }
    }

    void MkdirSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() >= 1,
            "fs.mkdirFileSync requires 1 parameters.");

        CHECK_ARG(isolate,
            args[0]->IsString(),
            "fs.mkdirFileSync requires a string as the 1st parameter for the directory.");
            
        v8::String::Utf8Value directory(args[0]);

        try {
            file_system_helpers::MkdirSync(std::string(*directory));
        } catch (const std::exception& ex) {
            isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
        }
    }

    void ExistsSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() >= 1,
            "fs.existsSync requires 1 parameters.");

        CHECK_ARG(isolate,
            args[0]->IsString(),
            "fs.existsSync requires a string as the 1st parameter for the path to check.");
            
        v8::String::Utf8Value path(args[0]);
        auto exists = file_system_helpers::ExistsSync(std::string(*path));

        args.GetReturnValue().Set(exists);
    }

    void ReaddirSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() >= 1,
            "fs.readdirSync requires 1 parameters.");

        CHECK_ARG(isolate,
            args[0]->IsString(),
            "fs.readdirSync requires a string as the 1st parameter for the path to check.");

        v8::String::Utf8Value directory(args[0]);
        auto names = file_system_helpers::ReadDirectorySync(std::string(*directory));

        auto context = isolate->GetCurrentContext();
        auto count = static_cast<uint32_t>(names.size());
        auto result = v8::Array::New(isolate, count);

        for (uint32_t i = 0; i < count; ++i) {
            (void)result->CreateDataProperty(context, i, v8_helpers::MakeV8String(isolate, names[i]));
        }
            
        args.GetReturnValue().Set(result);
    }

}   // End of anonymous namespace.