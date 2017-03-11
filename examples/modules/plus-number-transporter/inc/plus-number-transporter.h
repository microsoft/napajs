#pragma once

#include <napa/module/object-transport.h>

namespace napa {
namespace demo {

    /// <summary> Example class to show how to pass object pointer to a module. </summary>
    class PlusNumberTransporter : public module::TransportableObject<PlusNumberTransporter> {
    public:

        /// <summary> Constructor with initial value. </summary>
        explicit PlusNumberTransporter(double value = 0.0);

        /// <summary> Add the given value and return the result. </summary>
        double Add(double value);

    private:

        double _value;
    };

}  // napespace demo
}  // namespace napa
