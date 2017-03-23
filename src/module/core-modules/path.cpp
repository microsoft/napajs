#include "path.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>

#include <boost/filesystem.hpp>
#include <string>

using namespace napa;
using namespace napa::module;

namespace {

    /// <summary>
    /// Normalize a path by removing '.' and '..' and use preferred separator. (windows in \\ and posix in /).
    /// Example:
    /// path.normalize('c:/foo\\bar/.././baz/.')
    /// // returns 'c:\\foo\\baz'.
    /// </summary>
    /// <param name="args"> A sequence of paths to resolve. </param>
    void NormalizeCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Resolve a sequence of paths to one absolute path.
    /// Example:
    /// path.resolve('c:/foo\\bar', '.\\baz')
    /// // returns 'c:\\foo\\bar\\baz'.
    ///
    /// path.resolve('c:\\foo/bar', 'd:/tmp/file/')
    /// // returns 'd:\\tmp\\file'.
    ///
    /// path.resolve('wwwroot', 'static_files/png/', '../gif/image.gif')
    /// // if the current working directory is c:\\home\\myself\\node,
    /// // this returns 'c:\\home\\myself\\node\\wwwroot\\static_files\\gif\\image.gif'.
    /// </summary>
    /// <param name="args"> A sequence of paths to resolve. </param>
    void ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Parent directory name of a file name.
    /// Example:
    /// path.dirname('c:/foo\\bar\\baz/asdf\\quux')
    /// // returns 'c:\\foo\\bar\\baz\\asdf'.
    /// // even quux is a directory. The behavior is the same with Node.JS.
    /// </summary>
    /// <param name="args">1 Argument of file path. </param>
    void DirnameCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Base name of a file path.
    /// Example:
    /// path.basename('c:\\foo/bar/baz/asdf\\quux.html')
    /// // returns 'quux.html'.
    /// path.basename('c:/foo\\bar/baz/asdf/quux.html', '.html')
    /// // returns 'quux'.
    /// </summary>
    /// <param name="args"> 1 required argument of file path and another optional argument of extension. </param>
    void BasenameCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Extension of a file path.
    /// Example:
    /// path.extname('index.html')
    /// // returns '.html'.
    ///
    /// path.extname('index.coffee.md')
    /// // returns '.md'.
    ///
    /// path.extname('index.')
    /// // returns '.'.
    ///
    /// path.extname('index')
    /// // returns ''.
    /// </summary>
    /// <param name="args"> 1 argument of file path. </param>
    void ExtnameCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Check if a path is an absolute path or not.
    /// Example:
    /// path.isAbsolute('c:/foo/bar') // returns true.
    /// path.isAbsolute('c:\\baz/..')  // returns true.
    /// path.isAbsolute('qux/')     // returns false.
    /// path.isAbsolute('.')        // returns false.
    /// </summary>
    /// <param name="args"> 1 argument of file path. </param>
    void IsAbsoluteCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary>
    /// Get relative path from the first path to the second.
    /// Example:
    /// path.relative('c:/foo\\bar', 'c:/foo/abc.txt')
    /// // returns '..\\abc.txt'.
    ///
    /// path.relative('c:/foo\\../bar', 'c:/bar')
    /// // returns '.'.
    ///
    /// path.relative('c:\\foo', 'c:/')
    /// // returns '..'.
    ///
    /// path.relative('c:\\foo', 'd:\\bar')
    /// // returns 'd:\\bar'.
    /// </summary>
    /// <param name="args"> 2 strings represent from-path and to-path. </param>
    void RelativeCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

}   // End of anonymous namespace.

void path::Init(v8::Local<v8::Object> exports) {
    NAPA_SET_METHOD(exports, "normalize", NormalizeCallback);
    NAPA_SET_METHOD(exports, "resolve", ResolveCallback);
    NAPA_SET_METHOD(exports, "dirname", DirnameCallback);
    NAPA_SET_METHOD(exports, "basename", BasenameCallback);
    NAPA_SET_METHOD(exports, "extname", ExtnameCallback);
    NAPA_SET_METHOD(exports, "isAbsolute", IsAbsoluteCallback);
    NAPA_SET_METHOD(exports, "relative", RelativeCallback);
}

namespace {

    void NormalizeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsString() ,
            "path.normalize requires 1 string parameter of file path.");
        
        v8::String::Utf8Value utf8Path(args[0]);
        auto path = boost::filesystem::path(*utf8Path).normalize().make_preferred();
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, path.string()));
    }

    void ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() > 0,
            "path.resolve requires at least one string parameters.");

        auto path = boost::filesystem::current_path();

        for (int i = 0; i < args.Length(); ++i) {
            CHECK_ARG(isolate,
                args[i]->IsString(),
                "path.resolve doesn't accept non-string argument.");
            
            v8::String::Utf8Value nextPath(args[i]);
            path = boost::filesystem::absolute(boost::filesystem::path(*nextPath), path);
        }
        path.normalize().make_preferred();
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, path.string()));
    }

    void DirnameCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsString(),
            "path.dirname requires 1 string parameter of file path.");
        
        v8::String::Utf8Value utf8Path(args[0]);
        auto path = boost::filesystem::path(*utf8Path);
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate,
            path.has_parent_path() ?  path.parent_path().string() : path.string()));
    }

    void BasenameCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 || args.Length() == 2,
            "path.basename takes 1 required arugment of file path and 1 optional argument of extension");
        
        CHECK_ARG(isolate,
            args[0]->IsString() ,
            "path.basename requires a string parameter of file path.");
        
        v8::String::Utf8Value utf8Path(args[0]);
        auto fileName = boost::filesystem::path(*utf8Path).filename().string();
        if (args.Length() == 2) {
            CHECK_ARG(isolate,
                args[1]->IsString() ,
                "path.basename requires a string as 2nd parameter of extension.");

            v8::String::Utf8Value extension(args[1]);
            auto pos = fileName.find(*extension);
            if (pos != std::string::npos) {
                fileName = fileName.substr(0, pos);
            }
        }
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, fileName));
    }

    void ExtnameCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsString(),
            "path.extname requires 1 string parameter of file path.");
        
        v8::String::Utf8Value utf8Path(args[0]);
        auto path = boost::filesystem::path(*utf8Path);
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, path.extension().string()));
    }

    void IsAbsoluteCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsString() ,
            "path.isAbsolute requires 1 string parameter of file path.");
        
        v8::String::Utf8Value utf8Path(args[0]);
        auto path = boost::filesystem::path(*utf8Path);
        args.GetReturnValue().Set(path.is_absolute());
    }

    void RelativeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 2 && args[0]->IsString() && args[1]->IsString(),
            "path.relative requires 2 arguments of string type.");

        v8::String::Utf8Value from(args[0]);
        v8::String::Utf8Value to(args[1]);

        auto fromPath = boost::filesystem::absolute(*from).normalize().make_preferred();
        auto toPath = boost::filesystem::absolute(*to).normalize().make_preferred();
        auto relativePath = toPath.lexically_relative(fromPath);

        // from/to path doesn't share the same root (drive).
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, 
            relativePath.empty() ?  toPath.string() : relativePath.string()));
    }

}   // End of anonymous namespace.