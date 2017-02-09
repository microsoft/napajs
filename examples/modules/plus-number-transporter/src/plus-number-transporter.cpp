#include "plus-number-transporter.h"

using namespace napa::demo;

PlusNumberTransporter::PlusNumberTransporter(double value)
    : _value(value) {
}

double PlusNumberTransporter::Add(double value) {
    return _value + value;
}
