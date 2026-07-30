# Tempora-Fusion

## Verifiable Homomorphic Linear Combination Time-Locked Puzzle (VHLC-TLP)

Tempora-Fusion is a modular C++ implementation of the Verifiable Homomorphic
Linear Combination Time-Locked Puzzle protocol described in Section 5.2 of
*Tempora-Fusion: Time-Lock Puzzle with Efficient Verifiable Homomorphic Linear
Combination*.

The implementation realizes the high-level algorithms

```text
S.Setup, C.Setup, GenPuzzle, Evaluate, Solve, Verify
```

and includes the cryptographic building blocks used by those algorithms. The
provided executable is a fixed-parameter test and demonstration harness. It
does not currently expose a command-line interface, consume a dataset, or
generate paper-ready tables and plots.

## Evaluation road map

The estimates below are deliberately conservative and should be refined with a
measured reference machine before artifact submission.

| Stage | Reviewer action | Estimated time | Evidence |
|---|---|---:|---|
| 1. Inspect | Read this file and check package completeness | 5 human-min | All `.cpp` and corresponding `.h` files are present |
| 2. Install | Install `cryptopp-modern`, NTL, GMP, and a C++ compiler | 10--20 human-min + 5--20 compute-min | Dependency headers and libraries are discoverable |
| 3. Build | Compile `VHLC_TLP.exe` | 2 human-min + <2 compute-min | Executable is created with exit status 0 |
| 4. Run | Execute the default functional experiment | 2 human-min + up to 30 compute-min | `run.log` contains the checks listed below |
| 5. Validate | Compare the computed and reference values and inspect verification lines | 5--10 human-min | The acceptance conditions in [Expected results](#expected-results) hold |
| 6. Reuse | Change the experiment constants, rebuild, and rerun | 5--15 human-min plus runtime | A new random experiment completes under the chosen configuration |

Runtime depends strongly on CPU performance and cryptographic-library builds.
The default run generates ten client key pairs whose two RSA factors are each
2048 bits, and performs many OLE/OT operations. It is therefore normal for the
terminal to appear busy for several minutes.

## Package-integrity gate

The canonical repository must contain every `.cpp` file listed below and every
project header referenced by those translation units (for example, `setup.cpp`
includes `setup.h`). `TestRoutines.cpp` is the entry point and does not require
a same-named header.

If an artifact download contains only the supplied `.cpp` files, or if a file
service has renamed files to forms such as `setup(3).cpp`, the package is not
buildable as written. Restore the canonical names and obtain the missing
headers before continuing; this is a packaging error rather than a dependency
error.

The build also assumes that all public sample coordinates in `X` are distinct.
They are generated randomly by `Setup_S::generatePublicX()`, so a collision is
negligible but would cause interpolation to fail.

## Requirements

### Hardware

No special hardware, GPU, enclave, or network service is required. A
commodity 64-bit Linux machine is sufficient. For review, allocate:

- one or more CPU cores;
- at least 4 GB RAM;
- less than 1 GB free disk space for the project, dependencies, executable,
  and logs.

The implementation itself is mostly sequential. NTL may use threads depending
on how it was built.

### Software

- Linux or another Unix-like operating system;
- a C++17-capable compiler (GCC or Clang);
- [cryptopp-modern](https://github.com/cryptopp-modern/cryptopp-modern), not
  upstream Crypto++ 8.9, because `commitment.cpp` uses BLAKE3;
- [NTL](https://libntl.org/);
- [GMP](https://gmplib.org/);
- standard build tools.

On AArch64, use `cryptopp-modern` 2026.5.1 or later because earlier 2026
releases contained an architecture-specific BLAKE3 correctness defect.

No installation script is needed: these three dependencies are conventional
libraries and can be installed.

## Dependency installation

The following example targets Ubuntu 24.04. Package names may differ on other
systems.

```bash
sudo apt update
sudo apt install build-essential cmake git libntl-dev libgmp-dev
```

Install `cryptopp-modern` from its official repository. The project documents
both CMake and GNU Make builds; a typical CMake installation is:

```bash
git clone --depth 1 --branch 2026.7.1 \
  https://github.com/cryptopp-modern/cryptopp-modern.git
cd cryptopp-modern
cmake --preset=default
cmake --build build/default --parallel
./build/default/cryptest.exe v
sudo cmake --install build/default
sudo ldconfig
cd ..
```

These commands access the network, create a `cryptopp-modern/` source and build
tree, and install headers and libraries under the configured system prefix
(commonly `/usr/local`). They do not modify the Tempora-Fusion source tree.
If you already have a compatible build you may skip this step.

Confirm that the required BLAKE3 header is visible:

```bash
test -f /usr/local/include/cryptopp/blake3.h \
  || test -f /usr/include/cryptopp/blake3.h
```

No output and exit status 0 indicate success.

## Build

Run the following command from the directory containing the canonical source
and header filenames:

```bash
g++ -std=c++17 -O2 \
  gen_puzzle.cpp \
  ope_interface.cpp \
  poly_field.cpp \
  rsa.cpp \
  tlp.cpp \
  commitment.cpp \
  ole.cpp \
  OT_1of2.cpp \
  TestRoutines.cpp \
  helper_functions.cpp \
  ole_enhanced.cpp \
  prf.cpp \
  setup.cpp \
  coin_toss.cpp \
  linear_comb.cpp \
  poly_interpolate.cpp \
  solve_puzzles.cpp \
  verify.cpp \
  -L/usr/local/lib -lntl -lgmp -lcryptopp -pthread \
  -o VHLC_TLP.exe
```

If `cryptopp-modern` was installed elsewhere, add its include and library
directories with `-I<include-directory>` and `-L<library-directory>`, or replace
`-lcryptopp` with the absolute path to the installed static archive. Use one
Crypto++ linkage method, not both. The original implementation used
`/usr/local/lib/libcryptopp.a`.

Do not add `-DNDEBUG`: the test harness and protocol code use `assert` for
several precondition and correctness checks.

**Side effects.** The command reads the source and header files and creates or
overwrites only `VHLC_TLP.exe`. It does not access the network.

**Successful outcome.** The compiler exits with status 0 and
`test -x VHLC_TLP.exe` succeeds. A missing project header means the artifact
archive is incomplete. A missing `cryptopp/blake3.h` means upstream Crypto++,
rather than `cryptopp-modern`, was selected.

## Quick functional test

Run the fixed-parameter experiment and retain a human-readable log:

```bash
timeout 30m ./VHLC_TLP.exe 2>&1 | tee run.log
test "${PIPESTATUS[0]}" -eq 0
sed -r 's/\x1B\[[0-9;]*[mK]//g' run.log > run.clean.log
```

The executable accepts no command-line arguments. It uses fresh random primes,
keys, messages, coordinates, leaders, roots, and masks on every invocation, so
numeric values and timings will differ between runs.

**Side effects.** Execution is offline. The program writes only to standard
output; `tee` creates or overwrites `run.log`, and `sed` creates or overwrites
`run.clean.log`. Repeating the command is safe. The `timeout` bound prevents a
bad dependency build or unexpected performance problem from occupying the
review machine indefinitely.

Inspect the decisive lines:

```bash
grep -E \
  'PASS:|True sum:|PzlEval:|clientPzl:|Commitment check|Validity check|Verification for|All tests passed' \
  run.clean.log
```

## Expected results

A successful default run must satisfy **all** of the following:

1. The OT test reports:

   ```text
   PASS: All 1000 OT executions correct
   ```

2. The value printed after `True sum:` is identical to the value printed after
   `PzlEval:`. These are random field elements, so no fixed numeric answer is
   expected.

3. Every emitted `Commitment check ...` and `Validity check ...` line reports
   `Pass`.

4. The combined-puzzle verifier reports:

   ```text
   Verification for puzzle combination g: Accepted.
   ```

5. The individual-puzzle commitment check reports `Pass`, followed by:

   ```text
   Verification for puzzle o: Accepted.
   ```

6. The process exits with status 0 and ends with:

   ```text
   All tests passed.
   ```

The final banner alone is **not** sufficient evidence. In the current harness,
`testVHLCTLP()` stores but does not assert `g_outcome` or `o_outcome`.
Furthermore, `Verify::o_checkCommitments()` prints its result but does not set
the verifier flag on failure. Reviewers must therefore apply all six checks
above.

If a run fails because of random input or exceeds the time budget, preserve
`run.log`, report the environment and failing line, and rerun once. Do not
discard repeated failures as randomness.

## What the default experiment executes

`main()` in `TestRoutines.cpp` performs three active stages:

1. `testPolynomialGeneration()` creates a random 512-bit field prime,
   initializes NTL's process-global `ZZ_p` modulus, creates a random
   degree-one polynomial, and evaluates it.
2. `testOT()` executes 1,000 one-out-of-two transfers with choice bit 1 and
   128-bit OT keys, checking every selected message.
3. `testVHLCTLP(10, 3)` runs the complete Tempora-Fusion path for ten clients
   and three leaders, then solves and verifies the combined puzzle and client
   puzzle 0.

The end-to-end stage uses the following constants:

| Parameter | Default | Location | Meaning |
|---|---:|---|---|
| field size | 128-bit random prime | `testVHLCTLP()` | NTL prime field |
| clients | 10 | `main()` | number of client puzzles |
| leaders | 3 | `main()` | leaders chosen by the coin-toss output |
| client RSA parameter `lambda` | 2048 | `testVHLCTLP()` | bit length of each generated RSA prime factor |
| message size | 32 random bits | `testVHLCTLP()` | one message per client |
| client delay `delta[i]` | random integer 1 - 9 | `testVHLCTLP()` | per-client delay multiplier |
| `max_ss` | 1 | `testVHLCTLP()` | sequential-squaring scale |
| combination delay | 1 | `S_input.delta_combination` | evaluation-puzzle delay multiplier |
| coefficient `q` | 1 for every client | `C_input.q` | equal-weight sum |
| target client | 0 | `targetClient` | individual puzzle solved and verified |
| polynomial points | `t + 2 = 5` | `Setup_S::generatePublicX()` | point-value representation |

Thus, the demonstrated relation is

```text
PzlEval = m[0] + m[1] + ... + m[9] mod p.
```

The delay settings exercise sequential squaring but are intentionally too
small to substantiate real-time delay or performance claims.

Several additional routines exist but are commented out in `main()`:
`testOLE()`, `testOLE_enhanced()`, `testPRF()`, `testHash()`, and
`testCoinToss()`. The standalone RSA and TLP routines are also defined but not
called by the default executable. Uncomment only the desired calls, rebuild,
and state the change when reporting results.

## Claims and evidence

### C1: Protocol coverage

The source implements the Section 5.2 workflow from server/client setup through
puzzle generation, homomorphic evaluation, solving, and verification.

**Evidence:** successful construction and execution of the end-to-end objects
in `testVHLCTLP()`, ending in accepted combined and individual puzzles.

### C2: Correctness of the default equal-weight combination

For one independently generated ten-client instance, the solved combined
puzzle equals the direct modular sum of the ten random messages.

**Evidence:** equality of `True sum:` and `PzlEval:` in `run.clean.log`, plus
passing root and commitment checks and `Accepted.` from the combined verifier.

### C3: Exercisability and timing instrumentation

The implementation exposes per-phase wall-clock measurements for setup,
puzzle generation, leader/non-leader computation, combination, solving,
verification, and total protocol execution.

**Evidence:** lines ending in `execution time:` in `run.clean.log`. These values
describe one random run; they are not a statistically controlled benchmark.

### Claims not reproduced by this package

The present source package does not, by itself, reproduce formal security
proofs, calibrated real-time delays, asymptotic claims, comparisons with other
systems, or any unidentified paper table/figure. It provides no fixed random
seed, repeated-trial driver, structured results file, analysis script, or
plotting script.

## Architecture and source road map

### Protocol orchestration

| File | Responsibility and data flow |
|---|---|
| `setup.cpp` | `S.Setup` validates a field prime, initializes a degree-one `Poly_Field`, and samples `t+2` public coordinates. `C.Setup` generates an RSA modulus and totient per client. |
| `gen_puzzle.cpp` | `GenPuzzle` checks parameters; derives time-lock master keys; derives PRF keys and masks; encodes `pi_u(x)=x+m_u`; encrypts its point values; commits to each message; publishes puzzle vectors and public parameters. |
| `linear_comb.cpp` | `Evaluate` selects leaders, creates temporary time-lock keys and roots, generates zero-sum PRF masks, uses OLE+ to re-encode leader and non-leader puzzles, and sums their contributions into `g`. |
| `solve_puzzles.cpp` | Solves either an evaluated puzzle (`g_solve`) or an individual client puzzle (`o_solve`) by sequential squaring, PRF-based unblinding, interpolation, root extraction, and result publication. |
| `verify.cpp` | Checks openings and polynomial roots for evaluated puzzles, recomputes the result, and checks the individual-puzzle commitment. |
| `TestRoutines.cpp` | Contains the sole `main()`, active default experiment, inactive component tests, timing output, and all experiment constants. |

### Cryptographic and algebraic building blocks

| File | Responsibility |
|---|---|
| `rsa.cpp` | Generates two `lambda`-bit NTL primes, the RSA modulus `N=pq`, and `phi(N)`. |
| `tlp.cpp` | Demonstration RSA time-lock puzzle over string messages using trapdoor generation and sequential squaring for solving. |
| `prf.cpp` | AES-based counter-style expansion from a field input and integer key to a configurable-length NTL integer. |
| `commitment.cpp` | BLAKE3-based hashing overloads for `(x,r)` and `x`; used for message/root commitments and coin tossing. |
| `coin_toss.cpp` | Simulates committed random contributions by XOR and deterministically derives unique leader indices. |
| `OT_1of2.cpp` | GMP-based one-out-of-two transfer used by the OPE layer. |
| `ope_interface.cpp` | Builds random degree-one polynomials, prepares OT pairs, sums selected values, and extracts a polynomial evaluation. |
| `ole.cpp` | Implements oblivious evaluation of `a_0+a_1x` through the OPE interface. |
| `ole_enhanced.cpp` | Implements the two-OLE masked OLE+ transformation used during puzzle re-encoding. |
| `poly_field.cpp` | Initializes NTL's global `ZZ_p` modulus and provides degree-one polynomial sampling. |
| `poly_interpolate.cpp` | Performs Lagrange evaluation/interpolation, constructs a `ZZ_pX` polynomial, factors it, and returns its linear roots. |
| `helper_functions.cpp` | Converts strings, NTL integers, field elements, AES keys, and the message polynomial encoding. |

### Protocol data flow

```text
Setup_S + Setup_C
        |
        v
GenPuzzle: messages -> encoded point values -> blinded client puzzles o[u]
        |
        v
Evaluate: leader selection + OLE+ re-encoding -> combined puzzle g
        |
        +--------------------+
        v                    v
Solve g                  Solve o[0]
        |                    |
        v                    v
Verify g                 Verify o[0]
```

## Reusing the implementation

The reusable entry points are the C++ classes demonstrated in
`testVHLCTLP()`. There is no stable serialized file format or public CLI.

To evaluate a different configuration:

1. Edit the constants in `TestRoutines.cpp`.
2. Maintain `client_count > leader_count` as required by the construction.
3. Keep the server field prime at least 128 bits and the client setup parameter
   at least 2048 because the code enforces these bounds.
4. Ensure every coefficient `C_input.q`, message, coordinate, delay, and
   intermediate value is interpreted modulo the active NTL `ZZ_p` field where
   appropriate.
5. Increase `max_ss` or the delay multipliers only after estimating the number
   of sequential squarings. Runtime grows with those values.
6. Rebuild and rerun. Record the exact source revision, edited parameters,
   compiler flags, dependency versions, hardware, and raw log.
   
## Known limitations

- This is research prototype code, not a production cryptographic library.
- The executable has no CLI and uses compile-time experiment constants.
- Randomness is not seed-controlled, so exact values are not reproducible.
  Functional relations should be reproduced instead.
- `ZZ_p` has process-global modulus state. Constructing a `Poly_Field` changes
  that state; isolated component tests must initialize the intended field
  first.
- The default experiment validates a single equal-weight sum. It is not a
  parameter sweep or statistically controlled performance study.
- The final success banner does not assert the verifier return values; use the
  complete acceptance checklist above.
- The individual-puzzle verifier prints a commitment mismatch but does not
  currently propagate it to its decision flag. Require the explicit
  `Commitment check: Pass` line.
- The OT implementation allocates GMP arrays and matrices without complete
  per-call cleanup. Keep experiments bounded; process exit reclaims the memory.
- The commitment input concatenation does not encode component lengths. Treat
  it as the implementation of the paper prototype, not a general-purpose
  domain-separated commitment API.
- The coin-toss protocol is simulated within one process. It accounts for
  commitment work but does not model distributed communication or adversarial
  scheduling.
- No dataset, stored results, analysis scripts, plots, container, or virtual
  machine is included or required.

## Troubleshooting

### `fatal error: ... .h: No such file or directory`

The archive is missing project headers or filenames were changed during
download. Restore the complete canonical source tree.

### `fatal error: cryptopp/blake3.h: No such file or directory`

The compiler selected upstream Crypto++ or cannot see the
`cryptopp-modern` include directory. Install `cryptopp-modern` and add the
correct `-I` path if it is not under `/usr/local/include` or `/usr/include`.

### `cannot find -lcryptopp`, `-lntl`, or `-lgmp`

Add the installation library directory with `-L`, refresh the dynamic loader
cache where applicable, and verify that the compiler architecture matches the
libraries.

### Interpolation error or inverse-of-zero failure

Retain the log and rerun once. A rare duplicate random coordinate, zero mask,
or other degenerate random field value can invalidate an inversion. Repeated
failures indicate a defect or incompatible dependency build and should be
reported.

### Timeout

Record the CPU, available memory, dependency versions, and last completed
phase. Confirm that the defaults in `TestRoutines.cpp` were not increased.
Extending the timeout is reasonable only after confirming forward progress.

## Security, privacy, and ethics

The program generates synthetic random values only. It does not read personal
data, contact remote services, require privileged execution, or disable host
security controls. Building dependencies with `sudo cmake --install` modifies
the chosen system installation prefix; reviewers may instead install under an
unprivileged prefix and adjust `-I`, `-L`, and the runtime library path.

The program performs CPU-intensive cryptographic operations and the current OT
prototype retains allocations until process exit. Use the supplied timeout and
avoid running untrusted modifications with elevated privileges.

## References
- [cryptopp-modern documentation](https://cryptopp-modern.com/).
- [NTL installation documentation](https://libntl.org/doc/tour-unix.html).
- [GMP documentation](https://gmplib.org/manual/).
