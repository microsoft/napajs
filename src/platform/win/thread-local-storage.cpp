#include <napa/thread-local-storage.h>
#include <windows.h>

using namespace napa;

uint32_t tls::Alloc() {
    return TlsAlloc();
}

bool tls::Free(uint32_t index) {
    return TlsFree(index) != FALSE;
}

void* tls::GetValue(uint32_t index) {
    return TlsGetValue(index);
}

bool tls::SetValue(uint32_t index, void* value) {
    return TlsSetValue(index, value) != FALSE;
}