#include "specs.h"
#include <cstring>

namespace bitopt {
namespace {

uint8_t clearLowest(uint8_t x, uint8_t)  { return static_cast<uint8_t>(x & (x - 1)); }
uint8_t isolateLowest(uint8_t x, uint8_t){ return static_cast<uint8_t>(x & -x); }
uint8_t maskTrailing(uint8_t x, uint8_t) { return static_cast<uint8_t>(x ^ (x - 1)); }
uint8_t offTrailOnes(uint8_t x, uint8_t) { return static_cast<uint8_t>(x & (x + 1)); }
uint8_t setLowestZero(uint8_t x, uint8_t){ return static_cast<uint8_t>(x | (x + 1)); }
uint8_t propagateLow(uint8_t x, uint8_t) { return static_cast<uint8_t>(x | (x - 1)); }

uint8_t absSpec(uint8_t x, uint8_t) {
    int v = static_cast<int8_t>(x);
    return static_cast<uint8_t>(v < 0 ? -v : v);
}
uint8_t signSpec(uint8_t x, uint8_t) {
    int v = static_cast<int8_t>(x);
    return static_cast<uint8_t>(v == 0 ? 0 : (v < 0 ? -1 : 1));
}
uint8_t avgFloor(uint8_t x, uint8_t y) {
    return static_cast<uint8_t>((static_cast<int>(x) + static_cast<int>(y)) >> 1);
}

} // namespace

const std::vector<Spec>& allSpecs() {
    static const std::vector<Spec> specs = {
        {"clear_lowest",   "x & (x-1): clear lowest set bit",             1, {1}, clearLowest,   3},
        {"isolate_lowest", "x & -x: isolate lowest set bit",              1, {},  isolateLowest, 3},
        {"mask_trailing",  "x ^ (x-1): mask up to lowest set bit",        1, {1}, maskTrailing,  3},
        {"off_trail_ones", "x & (x+1): clear trailing ones",              1, {1}, offTrailOnes,  3},
        {"set_lowest_zero","x | (x+1): set lowest zero bit",              1, {1}, setLowestZero, 3},
        {"propagate_low",  "x | (x-1): propagate lowest set bit",         1, {1}, propagateLow,  3},
        {"abs",            "|x| for 8-bit two's complement",              1, {7}, absSpec,       4},
        {"sign",           "-1 / 0 / +1 by sign of x",                    1, {7}, signSpec,      4},
        {"avg_floor",      "floor((x+y)/2) without overflow (unsigned)",  2, {1}, avgFloor,      4},
    };
    return specs;
}

const Spec* findSpec(const char* name) {
    for (const auto& s : allSpecs())
        if (std::strcmp(s.name, name) == 0) return &s;
    return nullptr;
}

} // namespace bitopt