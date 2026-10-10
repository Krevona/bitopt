#include "search.h"
#include <cstdio>

using namespace bitopt;

int main() {
    int failures = 0;
    for (const auto& s : allSpecs()) {
        Result r = synthesize(s, s.budget);
        bool ok = r.found && r.verified && r.cost <= s.budget;
        std::printf("[%s] %-16s cost=%d shared=%d\n", ok ? " ok " : "FAIL", s.name, r.cost, r.dagSize);
        if (!ok) ++failures;
    }
    return failures ? 1 : 0;
}