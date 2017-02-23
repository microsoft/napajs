#include <catch.hpp>

#include <napa/object-transport.h>

using namespace napa;
using namespace napa::module;

class Vehicle {};
class Car : public Vehicle {};
class Sky {};

TEST_CASE("object-transporter serializes and deserializes a stack object correctly.", "[object-transporter]") {
    Car car;
    auto transporter = ObjectTransporter::Serialize<Car>(&car);

    SECTION("object-transporter deserializes an object correctly.") {
        auto object = ObjectTransporter::Deserialize<Car>(transporter.first);
        REQUIRE(object != nullptr);
        REQUIRE(object.get() == &car);
    }

    SECTION("object-transporter failes on a parent object.") {
        auto object = ObjectTransporter::Deserialize<Vehicle>(transporter.first);
        REQUIRE(object == nullptr);
    }

    SECTION("object-transporter failes on different type of object.") {
        auto object = ObjectTransporter::Deserialize<Sky>(transporter.first);
        REQUIRE(object == nullptr);
    }
}

TEST_CASE("object-transporter serializes and deserializes an heap object correctly.", "[object-transporter]") {
    auto vehicle = std::make_shared<Vehicle>();
    auto transporter = ObjectTransporter::Serialize<Vehicle>(vehicle);

    SECTION("object-transporter deserializes an object correctly.") {
        auto object = ObjectTransporter::Deserialize<Vehicle>(transporter.first);
        REQUIRE(object != nullptr);
        REQUIRE(object.get() == vehicle.get());
    }

    SECTION("object-transporter failes on a child object.") {
        auto object = ObjectTransporter::Deserialize<Car>(transporter.first);
        REQUIRE(object == nullptr);
    }

    SECTION("object-transporter failes on different type of object.") {
        auto object = ObjectTransporter::Deserialize<Sky>(transporter.first);
        REQUIRE(object == nullptr);
    }

    SECTION("object-transporter deserializes correctly after Vehicle instance is destroyed.") {
        auto pointer = vehicle.get();
        vehicle.reset();
        auto object = ObjectTransporter::Deserialize<Vehicle>(transporter.first);
        REQUIRE(object != nullptr);
        REQUIRE(object.get() == pointer);
    }
}

class Portable : public TransportableObject<Portable> {};

TEST_CASE("object-transporter serializes and deserializes an TransportableObject object correctly.", "[object-transporter]") {
    auto portable = std::make_shared<Portable>();
    auto transporter = portable->Serialize();

    SECTION("object-transporter deserializes an object correctly.") {
        auto object = ObjectTransporter::Deserialize<Portable>(transporter.first);
        REQUIRE(object != nullptr);
        REQUIRE(object.get() == portable.get());
    }

    SECTION("object-transporter failes on different type of object.") {
        auto object = ObjectTransporter::Deserialize<Car>(transporter.first);
        REQUIRE(object == nullptr);
    }

    SECTION("object-transporter deserializes correctly after Car instance is destroyed.") {
        auto pointer = portable.get();
        portable.reset();
        auto object = ObjectTransporter::Deserialize<Portable>(transporter.first);
        REQUIRE(object != nullptr);
        REQUIRE(object.get() == pointer);
    }
}

TEST_CASE("object-transport converts correctly between pointer value and array.", "[object-transporter]") {
    auto car = std::make_unique<Car>();

    auto source = object_transport::UintptrToUint32Array(reinterpret_cast<uintptr_t>(car.get()));
    auto target = object_transport::Uint32ArrayToUintptr(source);

    REQUIRE(car.get() == reinterpret_cast<Car*>(target));
}