#pragma once
#include <cstdint>
#include <vector>

namespace bitopt {

struct Spec {
    const char* name;
    const char* description;
    int arity;                      // 1 or 2 inputs (x, y)
    std::vector<uint8_t> consts;    // constants the search may use as free leaves
    uint8_t (*fn)(uint8_t, uint8_t);
    int budget;                     // max program length searched by default
};

const std::vector<Spec>& allSpecs();
const Spec* findSpec(const char* name);

} // namespace bitopt