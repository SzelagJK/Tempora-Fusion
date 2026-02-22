## Tempora-Fusion  
### Verifiable Homomorphic Linear Combination Time-Locked Puzzle (VHLC-TLP)

This repository implements **Tempora-Fusion**, a Verifiable Homomorphic Linear Combination Time-Locked Puzzle protocol as defined in Section 5.2 of the paper *“Time-Lock Puzzle with Efficient Verifiable Homomorphic Linear Combination”*.

Tempora-Fusion realizes the formal VHLC-TLP syntax:


S.Setup, C.Setup, GenPuzzle, Evaluate, Solve, Verify


using a modular C++ implementation structured around cryptographic building blocks and higher-level orchestration components.

The implementation is named:

**Tempora-Fusion: Verifiable Homomorphic Linear Combination Time Locked Puzzle**

---

## Architectural Overview

The implementation strictly follows the layered construction described in Section 5.2 of the paper.

### Layer 1 — Cryptographic Primitives (Building Blocks)

These files implement the abstract primitives assumed in the formal model:

| Primitive | File | Role Summary |
|------------|------|------------------|
| RSA-based TLP | `rsa.cpp`, `tlp.cpp` | Sequential squaring time-lock mechanism |
| PRF | `prf.cpp` | Derives blinding factors from master keys |
| Commitment (Hash-based) | `commitment.cpp` | Binding of message/root to puzzle randomness |
| OT (1-out-of-2) | `OT_1of2.cpp` | Base OT primitive |
| OLE | `ole.cpp` | Oblivious linear function evaluation |
| OLE+ | `ole_enhanced.cpp` | Verified blinding-factor switching |
| Coin Toss | `coin_toss.cpp` | Random leader/root generation |
| Polynomial Field Arithmetic | `poly_field.cpp` | Arithmetic over F_p |
| Polynomial Interpolation | `poly_interpolate.cpp` | Lagrange interpolation in point-value form |
| OPE Interface | `ope_interface.cpp` | Abstraction for polynomial encryption handling |

These components *with their corresponding header files*, map directly to the preliminaries of the protocol.

---

### Layer 2 — Protocol Construction (Section 5.2 Mapping)

These modules compose primitives into the Tempora-Fusion protocol:

| Protocol Algorithm | File |
|-------------------|------|
| Setup (S.Setup) | `setup.cpp` |
| Setup (C.Setup) | `setup.cpp` |
| GenPuzzle | `gen_puzzle.cpp` |
| Evaluate (Linear Combination) | `linear_comb.cpp` |
| Solve | `solve_puzzles.cpp` |
| Verify | `verify.cpp` |
| Test Harness | `TestRoutines.cpp` |

These implement the exact algorithmic workflow described in Section 5.2.

---

## Abstraction Mapping

This section explains how the implementation maps each abstract primitive into code.

---

### RSA-Based Time Lock Puzzle (TLP)

**Files:** `rsa.cpp`, `tlp.cpp`

Implements the sequential modular squaring function:

`mk = (r^2)^T mod N`

Mapping:

- `rsa.cpp` → RSA key generation and arithmetic  
- `tlp.cpp` → Sequential squaring execution  
- `solve_puzzles.cpp` → Invokes solving procedure  

The master key `mk` is used exclusively to derive PRF outputs.

---

### PRF (Pseudorandom Function)

**File:** `prf.cpp`

Implements:

`PRF : {0,1}* x {0,1}^poly(lambda) >> F ` 
(takes value x, and a key, outputs arbitrary length pseudorandom value)


Used to derive:

- Blinding factors
- Multiplicative masks 



---

### Commitment Scheme

**File:** `commitment.cpp`

Implements:


`Com(x, r)`


Used for:

- Committing to solution  
- Committing to leader root values  

Randomness is the master key (`mk` or `tk_u`), enabling opening only after solving.

---

### Polynomial Representation

**Files:** `poly_field.cpp`, `poly_interpolate.cpp`

Implements:

- Field arithmetic over `F_p`  
- Point-value representation of polynomials  
- Lagrange interpolation (evaluation, polynomial interpolation, root interpolation) 

Messages are encoded as a polynomial `pi(x)` in point-value form.

Encrypted representation:

`o_(i,u) = w_(i,u) * pi_(i,u) + z_(i,u) mod p`

This logic is then implemented in `gen_puzzle.cpp`.

---

### OLE and OLE+

**Files:** `ole.cpp`, `ole_enhanced.cpp`, `OT_1of2.cpp`

Implements:

`s = ac + b`

OLE+ is used for:

- Secure switching of blinding factors  
- Injecting leader roots  
- Ensuring zero-sum masking  
- Enforcing evaluation correctness  

Used in:

`linear_comb.cpp`

This directly implements the Evaluate algorithm of Section 5.2.

---

### Coin Toss

**File:** `coin_toss.cpp`

Used for:

- Collectively choosing random key and selection  

Ensures unpredictability of evaluation structure.

---

## Protocol Phase Mapping

This section shows how high-level protocol steps map to source files.

---

### Setup — `setup.cpp`

Implements `S.Setup(1^lambda, t, t_dash)`

Generates:

- Large prime `p`  
- Set `(X = \{x_1, ..., x_(t_dash))`

Matches the protocol Setup stage.

---

### Puzzle Generation — `gen_puzzle.cpp`

Implements `GenPuzzle(m_u, K_u, pks, delta_u, maxss)`

Steps:

1. Generate RSA modulus `N_u` 
2. Compute master key via TLP  
3. Derive PRF values  
4. Encode message as polynomial  
5. Encrypt point-values  
6. Commit to solution  

Produces:

- Puzzles vector `o_u`  
- Public parameters  
- Commitment  

---

### Linear Combination — `linear_comb.cpp`

Implements `Evaluate()`

Responsibilities:

- Leader selection  
- Root generation  
- Temporary TLP creation  
- Blinding factor switching via OLE+  
- Zero-sum masking  
- Component-wise summation  

Produces a vector:

`g = [g_1, ..., g_(t_dash)]`

Which is a polynomial encoding of the linear combination 

---

### Solving — `solve_puzzles.cpp`

Handles two cases:

- `clientPzl`  
- `evalPzl`  

Steps:

1. Solve required TLP(s)  
2. Remove blinding layers  
3. Interpolates polynomial  
4. Extracts:
   - Solution  
   - Roots  
   - Commitments

---

### Verification — `verify.cpp`

Implements `Verify()`

Checks:

- Commitment openings  
- Root presence in polynomial  
- Consistency of decrypted structure  

Implements solution-validity requirements of VHLC-TLP.

---

## Default Behavior

By default, the protocol executes unit tests.

**Test suite is located in TestRoutines.cpp**

Which validate:

- TLP correctness  
- PRF determinism  
- OT correctness
- OLE/OLE+ integrity  
- Polynomial encoding consistency  
- Commitment
- End-to-end evaluation and verification  
- **Runs the entire Tempora-Fusion protocol with unit tests**

---

## Compilation

To compile the entire Tempora-Fusion protocol:

```bash
g++ gen_puzzle.cpp ope_interface.cpp poly_field.cpp rsa.cpp tlp.cpp commitment.cpp ole.cpp OT_1of2.cpp TestRoutines.cpp helper_functions.cpp ole_enhanced.cpp prf.cpp setup.cpp coin_toss.cpp linear_comb.cpp poly_interpolate.cpp solve_puzzles.cpp verify.cpp /usr/local/lib/libcryptopp.a -lntl -lgmp -lcryptopp -o VHLC_TLP.exe
```

### Important

- `/usr/local/lib/libcryptopp.a` must point to a static library built from **modern-cryptopp**
- The implementation requires modern **Crypto++** due to BLAKE3 hash usage

### External Dependencies

- Crypto++ (modern-cryptopp)
- NTL
- GMP


