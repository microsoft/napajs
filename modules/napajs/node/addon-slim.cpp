#include <napa/module/shared-wrap.h>
#include <napa/module/allocator-wrap.h>
#include "transport-context-wrap-impl.h"
#include "simple-allocator-debugger-wrap.h"

using namespace napa::binding;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR();
NAPA_DEFINE_PERSISTENT_ALLOCATOR_WRAP_CONSTRUCTOR();

void InitAll(v8::Local<v8::Object> exports) {
    NAPA_SHARED_WRAP_INIT();
    NAPA_ALLOCATOR_WRAP_INIT();

    napa::binding::TransportContextWrapImpl::Init();
    napa::binding::SimpleAllocatorDebuggerWrap::Init();

    NAPA_EXPORT_OBJECTWRAP(exports, "SharedWrap", napa::module::SharedWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorWrap", napa::module::AllocatorWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "TransportContextWrap", napa::binding::TransportContextWrapImpl);
    NAPA_EXPORT_OBJECTWRAP(exports, "SimpleAllocatorDebuggerWrap", napa::binding::SimpleAllocatorDebuggerWrap);

}

NAPA_MODULE(addon, InitAll)
