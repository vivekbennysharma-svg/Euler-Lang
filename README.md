"# Euler-Lang" 

## Current Progress (Phase 1: Proof of Concept)

- [x] **File I/O Engine**: Reads `.eul` source files directly from disk into an in-memory character stream using pure C standard file I/O.
- [x] **Custom Lexer & Tokenizer**: Scans source text to tokenize identifiers, floating-point and integer numbers, mathematical operators (`+`, `-`, `*`, `/`, `^`), and grouping symbols.
- [x] **Recursive Descent Parser**: Implemented operator precedence parsing (PEMDAS-compliant) with right-associative power (`^`) and function call handling into an Abstract Syntax Tree (AST).
- [x] **AST Code Generator**: Walks the syntax tree and emits optimized, valid C code interfacing with runtime constructors (`euler_num`, `euler_var`, `euler_add`, `euler_pow`, etc.).
- [x] **Symbolic Calculus Runtime (`libeuler`)**:
  - Full expression tree data structure supporting variables, numbers, binary operators, and trigonometric functions (`sin`, `cos`).
  - Symbolic differentiation engine implementing the power rule, sum/difference rules, product rule, quotient rule, and trigonometric chain rules.
  - Formatted expression pretty-printer.
- [x] **Automated Build Toolchain**:
  - `Makefile` configuring the complete pipeline via `mingw32-make`.
  - Windows CLI runner script (`run.bat`) allowing one-command execution (`run test.eul`).
  - Automatic intermediate C generation, GCC compilation, link-time cleanup, and execution.

---

## Roadmap & Next Steps

### Phase 1b: Algebraic Simplification Engine
- [ ] **Trivial Identity Elimination**:
  - Additive identities: $x + 0 \to x$, $x - 0 \to x$.
  - Multiplicative identities: $x \times 1 \to x$, $x \times 0 \to 0$, $x / 1 \to x$.
  - Exponent identities: $x^1 \to x$, $x^0 \to 1$.
- [ ] **Constant Folding**: Pre-evaluate operations with pure numeric operands at compile/evaluation time (e.g., $3 \times 2 \to 6$).

### Phase 2: Full Program Scope & Variable Binding
- [ ] **Multi-line Statement Parser**: Support statements terminated by semicolons (`;`).
- [ ] **Variable Assignment**: Support expressions assigned to named variables (`expr f = x^2 + 3;`).
- [ ] **Symbol Table / Environment**: Track bound expressions and substitute variables during evaluation.

### Phase 3: Advanced Calculus & Linear Algebra
- [ ] **Limits & Definite Integration**: Implement numerical quadratures (Simpson's / Gauss-Legendre) and basic limit evaluations.
- [ ] **Matrix Syntax Primitives**: Implement matrix literals (e.g., `[[1, 2], [3, 4]]`) and basic linear solvers ($A \cdot X = B$).
