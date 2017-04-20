
#include <napa/module/shared-wrap.h>
#include "transport-context-wrap-impl.h"

using namespace napa::binding;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR;

void InitAll(v8::Local<v8::Object> exports) {
    NAPA_SHARED_WRAP_INIT();

    napa::binding::TransportContextWrapImpl::Init();

    NAPA_EXPORT_OBJECTWRAP(exports, "SharedWrap", napa::module::SharedWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "TransportContextWrap", napa::binding::TransportContextWrapImpl);
}

NAPA_MODULE(addon, InitAll)
