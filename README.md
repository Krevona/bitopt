# bitopt

bitopt searches for short instruction sequences that implement bit-manipulation
functions over the full 8-bit input domain. It requires a C++17 compiler.

## Build and test

```sh
g++ -O2 -std=c++17 -Wall -Wextra -Isrc src/main.cpp src/search.cpp src/specs.cpp -o bitopt
g++ -O2 -std=c++17 -Isrc tests/test_specs.cpp src/search.cpp src/specs.cpp -o test_specs
./test_specs
```

On Windows, use `.exe` output names and run executables with `.\`, for example
`.\bitopt.exe`.

## Usage

```sh
./bitopt --list
./bitopt --spec abs
./bitopt --spec clear_lowest --max-len 4
./bitopt --all
```

`--list` shows available targets, `--spec` searches one target, and `--all`
searches every target. `--max-len` overrides the default search budget.

Search uses 8-bit arithmetic and verifies candidates across the full input
domain. For two-input targets, candidates are generated from samples, so the
search may miss a valid solution within the budget.
