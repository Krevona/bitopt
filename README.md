# bitopt

bitopt is an enumerative superoptimizer for small bit-manipulation functions.
It searches for short C-like instruction sequences that match a target function
over the complete 8-bit input domain.

## Requirements

- A C++17 compiler. GCC is used in the examples; MinGW-w64 GCC works on Windows.
- CMake is not required.

## Build

From the project root, build the command-line program:

```sh
g++ -O2 -std=c++17 -Wall -Wextra -Isrc src/main.cpp src/search.cpp src/specs.cpp -o bitopt
```

On Windows with MinGW, use an `.exe` output name:

```powershell
g++ -O2 -std=c++17 -Wall -Wextra -Isrc src/main.cpp src/search.cpp src/specs.cpp -o bitopt.exe
```

Build and run the test program separately:

```sh
g++ -O2 -std=c++17 -Isrc tests/test_specs.cpp src/search.cpp src/specs.cpp -o test_specs
./test_specs
```

On Windows, change the output name to `test_specs.exe` and run it with
`.\test_specs.exe`.

## Usage

```sh
./bitopt --list
./bitopt --spec abs
./bitopt --spec clear_lowest --max-len 4
./bitopt --all
```

On Windows PowerShell, invoke the executable as `.\bitopt.exe` instead of
`./bitopt`. `--list` prints the available target functions. `--spec <name>`
searches one target, and `--max-len N` overrides its default search budget.
`--all` searches every target and prints a Markdown table of the results.

## Targets

The built-in targets include bit tricks such as clearing, isolating, or
propagating the lowest set bit; signed 8-bit absolute value and sign; and the
floor average of two unsigned bytes.

## Search model

- Values are 8-bit unsigned integers; arithmetic wraps modulo 256.
- The instruction set is `not`, `neg`, `add`, `sub`, `and`, `or`, `xor`,
  `shl`, `shr`, and `sar`. Shift counts are masked to their low three bits;
  `sar` shifts a signed 8-bit value arithmetically.
- Search proceeds bottom-up by expression-tree cost. Constants are restricted
  to those declared for each target.
- Equivalent sampled output vectors are pruned, and the final search level is
  scanned without storing all candidates to reduce memory use.
- A candidate solution is verified across the full domain: all 256 values for
  one-input targets, or all 65,536 input pairs for two-input targets.
- Reported tree cost counts repeated subexpressions each time they are used;
  the printed instruction sequence shares identical subexpressions.

## Limitations

- A result is verified only for 8-bit semantics; it is not proof that the same
  expression works for 32- or 64-bit values.
- Two-input search uses 256 sampled input pairs for candidate generation and
  pruning. Full verification prevents returning an incorrect result, but
  sample-based pruning can discard a useful candidate, so the search is not
  guaranteed to find every solution within the budget.
- The cost model counts operations only; it does not model latency,
  throughput, or target-processor-specific instruction costs.
