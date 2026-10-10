#pragma once
#include "specs.h"
#include <cstddef>
#include <string>

namespace bitopt {

struct Result {
    bool found = false;
    bool verified = false;          // checked on the full 8-bit input domain
    int cost = -1;                  // expression-tree size (ops)
    int dagSize = 0;                // instructions after sharing identical subexpressions
    std::string program;
    size_t nodesStored = 0;         // distinct value-vectors kept
    size_t candidatesEvaluated = 0;
    size_t falseFits = 0;           // sample matched but full verification failed
    double seconds = 0.0;
};

Result synthesize(const Spec& spec, int maxLen);

} // namespace bitopt