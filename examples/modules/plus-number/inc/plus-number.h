// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#ifdef NAPA_EXAMPLE_API
#define NAPA_EXAMPLE_EXPORT __declspec(dllexport)
#else
#define NAPA_EXAMPLE_EXPORT __declspec(dllimport)
#endif

namespace napa {
namespace demo {

    /// <summary> Example class to show how to create Napa module using a wrapped C++ class. </summary>
    class NAPA_EXAMPLE_EXPORT PlusNumber {
    public:
        /// <summary> Constructor with initial value. </summary>
        explicit PlusNumber(double value = 0.0);

        /// <summary> Add the given value and return the result. </summary>
        double Add(double value);

    private:
        double _value;
    };

} // napespace demo
} // namespace napa
