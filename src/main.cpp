#include "search.h"
#include "specs.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace bitopt;

static void usage() {
    std::puts("usage: bitopt --list\n"
              "       bitopt --spec <name> [--max-len N]\n"
              "       bitopt --all   (prints a markdown results table)");
}

static void runOne(const Spec& s, int maxLen) {
    Result r = synthesize(s, maxLen > 0 ? maxLen : s.budget);
    std::printf("spec: %s  (%s)\n", s.name, s.description);
    if (!r.found) {
        std::printf("no program found within length %d  [%zu nodes, %zu candidates, %.3fs]\n",
                    maxLen > 0 ? maxLen : s.budget, r.nodesStored, r.candidatesEvaluated, r.seconds);
        return;
    }
    std::printf("found: %d ops (tree), %d instructions (shared); verified on the full 8-bit domain\n%s",
                r.cost, r.dagSize, r.program.c_str());
    std::printf("search: %zu nodes stored, %zu candidates, %.3fs\n",
                r.nodesStored, r.candidatesEvaluated, r.seconds);
}

int main(int argc, char** argv) {
    const char* spec = nullptr;
    int maxLen = -1;
    bool all = false, list = false;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--spec") && i + 1 < argc) spec = argv[++i];
        else if (!std::strcmp(argv[i], "--max-len") && i + 1 < argc) maxLen = std::atoi(argv[++i]);
        else if (!std::strcmp(argv[i], "--all")) all = true;
        else if (!std::strcmp(argv[i], "--list")) list = true;
        else { usage(); return 1; }
    }
    if (list) {
        for (const auto& s : allSpecs()) std::printf("%-16s %s\n", s.name, s.description);
        return 0;
    }
    if (all) {
        std::puts("| spec | ops (tree) | instrs (shared) | verified | nodes stored | candidates | time (s) |");
        std::puts("|---|---|---|---|---|---|---|");
        for (const auto& s : allSpecs()) {
            Result r = synthesize(s, maxLen > 0 ? maxLen : s.budget);
            std::printf("| %s | %d | %d | %s | %zu | %zu | %.3f |\n", s.name, r.cost, r.dagSize,
                        r.verified ? "yes" : "NO", r.nodesStored, r.candidatesEvaluated, r.seconds);
        }
        return 0;
    }
    if (spec) {
        const Spec* s = findSpec(spec);
        if (!s) { std::fprintf(stderr, "unknown spec '%s' (try --list)\n", spec); return 1; }
        runOne(*s, maxLen);
        return 0;
    }
    usage();
    return 1;
}