#ifndef NAPA_SETTINGS_H
#define NAPA_SETTINGS_H

#include <string>

namespace napa {
namespace runtime {
namespace internal {

    class Settings {
    public:
        void Parse(const std::string& settings);
    };
}
}
}

#endif //NAPA_SETTINGS_H
