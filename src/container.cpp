#include "container.h"

using namespace napa::runtime::internal;

void Container::Initialize(Settings&& settings) {
    _settings = std::move(settings);
}
