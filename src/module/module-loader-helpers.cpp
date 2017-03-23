#include "module-loader-helpers.h" 
#include "core-modules/file-system-helpers.h"

#include <napa/v8-helpers.h>

#include <boost/dll.hpp>

using namespace napa;
using namespace napa::module;

namespace {

    std::string _moduleRootDirectory = boost::dll::this_line_location().parent_path().string();

}   // End of anonymous namespace.

v8::Local<v8::Object> module_loader_helpers::ExportModule(v8::Local<v8::Object> object,
                                                          const napa::module::ModuleInitializer& initializer) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto module = object->Get(v8_helpers::MakeV8String(isolate, "module"))->ToObject();
    auto exports = module->Get(v8_helpers::MakeV8String(isolate, "exports"))->ToObject();
    if (initializer != nullptr) {
        initializer(exports, object);
    }

    return scope.Escape(exports);
}

std::string module_loader_helpers::GetCurrentContextDirectory() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();
    auto contextObject = context->Global();
    auto dirPropertyName = v8_helpers::MakeV8String(isolate, "__dirname");

    if (contextObject.IsEmpty() 
        || contextObject->IsNull()
        || !contextObject->Has(dirPropertyName)) {
        return _moduleRootDirectory;
    }

    v8::String::Utf8Value callingPath(contextObject->Get(dirPropertyName));
    return std::string(*callingPath);
}

void module_loader_helpers::SetContextModulePath(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "__dirname"),
                                      v8_helpers::MakeV8String(isolate, _moduleRootDirectory.c_str()));
    (void)exports->CreateDataProperty(context, v8_helpers::MakeV8String(isolate, "__filename"), v8::Null(isolate));
}

v8::Local<v8::Context> module_loader_helpers::SetUpModuleContext(const std::string& path) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    // Create module object following the node module algorithm.
    auto module = v8::ObjectTemplate::New(isolate);
    auto exports = v8::Object::New(isolate);

    (void)exports->CreateDataProperty(isolate->GetCurrentContext(),
                                      v8_helpers::MakeV8String(isolate, "exports"),
                                      v8::Object::New(isolate));
    
    module->Set(v8_helpers::MakeV8String(isolate, "module"), exports);
    module->Set(v8_helpers::MakeV8String(isolate, "exports"),
                exports->Get(v8_helpers::MakeV8String(isolate, "exports"))->ToObject());

    if (path.empty()) {
        (void)module->Set(v8_helpers::MakeV8String(isolate, "__dirname"),
                            v8_helpers::MakeV8String(isolate, _moduleRootDirectory.c_str()));
        (void)module->Set(v8_helpers::MakeV8String(isolate, "__filename"), v8::Null(isolate));
    } else {
        boost::filesystem::path current(path);

        (void)module->Set(v8_helpers::MakeV8String(isolate, "__dirname"),
                            v8_helpers::MakeV8String(isolate, current.parent_path().string().c_str()));
        (void)module->Set(v8_helpers::MakeV8String(isolate, "__filename"),
                            v8_helpers::MakeV8String(isolate, path.c_str()));
    }

    // Create a sandbox to load javascript module.
    auto context = v8::Context::New(isolate, nullptr, module);
    JS_ENSURE_WITH_RETURN(isolate,
                          !context.IsEmpty(),
                          "Can't create module context for: " + path,
                          scope.Escape(context));

    return scope.Escape(context);
}

v8::Local<v8::String> module_loader_helpers::ReadModuleFile(const std::string& path) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    std::string content;
    try {
        content = file_system_helpers::ReadFileSync(path);
    } catch (const std::exception& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
        return scope.Escape(v8::Local<v8::String>());
    }

    JS_ENSURE_WITH_RETURN(isolate,
                          !content.empty(),
                          path + " is empty",
                          scope.Escape(v8::Local<v8::String>()));

    return scope.Escape(v8_helpers::MakeV8String(isolate, content));
}
