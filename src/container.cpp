#include "container.h"

using namespace napa::runtime::internal;

void Container::Initialize(std::unique_ptr<Settings> settings) {
    _settings = std::move(settings);
}
