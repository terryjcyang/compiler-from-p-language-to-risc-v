# hw5 report

|      |           |
| ---: | :-------- |
| Name | 楊睿宸    |
|   ID | 112550049 |

## How much time did you spend on this project

**~35 hours total:**

- **Spec & Tutorial (~5 hours):** Studying the RISC-V assembly manual and the provided stack machine model.
- **Implementation (~25 hours):** Developing the `CodeGenerator` visitor, managing stack offsets, and implementing bonus types.
- **Testing & Debugging (~5 hours):** Validating output assembly with `spike` and the `riscv32-unknown-elf-gcc` compiler.

## Project overview

This project implements a **Code Generator** that translates a P language AST into **RISC-V assembly**. The implementation follows a **stack machine model** and uses the **Visitor Pattern** to traverse the AST.

### 1. Stack Machine Architecture

The core logic relies on using the stack pointer (`sp`) to evaluate expressions:

- **Variable References:** Push the value (RHS) or address (LHS) onto the stack.
- **Operators:** Pop operands into temporary registers (`t0`, `t1`), perform the operation, and push the result back.
- **Assignment:** Pop the value and address, then store the value into that memory location.

### 2. Memory and Scope Management

- **Global Symbols:** Declared in `.bss` (uninitialized variables) or `.rodata` (constants) sections using `.comm` or `.word` directives.
- **Local Symbols:** Managed within a fixed **128-byte stack frame** for each function. Local variables are accessed via offsets relative to the frame pointer (`s0`).
- **Symbol Tables:** Reused from previous assignments to track the level and memory offset of each identifier.

### 3. Control Flow and Functions

- **Control Flow:** Implemented using a label generator (`nextL`) to manage unique branch targets for `if`, `while`, and `for` statements.
- **Function Calls:** Adheres to the RISC-V calling convention. Parameters are passed via `a0-a7` (integers) and `fa0-fa7` (reals). Arguments exceeding these registers are handled on the caller's stack.

### 4. Bonus Implementation

- **Boolean:** Treated as integer literals `1` (true) and `0` (false).
- **String:** Literals are stored in the `.rodata` section. Variables store and pass the memory address of these strings.
- **Real (Floating Point):** Utilizes RISC-V floating-point registers (`ft0`, `fa0`) and instructions like `flw`, `fsw`, and `fadd.s`.

## What is the hardest you think in this project

### Primary Challenge: Boilerplate and Management

The most difficult aspect was managing the sheer volume of assembly boilerplate code. Ensuring that every visitor emitted the correct sequence of instructions while maintaining a consistent stack state required constant cross-referencing with the RISC-V specification.

### Technical Difficulty: Efficiency vs. Readability

As noted in my sketch, balancing the efficiency of the generated code with its readability for debugging was a significant hurdle. I had to carefully design the stack machine flow so that the emitted assembly remained human-traceable while correctly handling complex operations like nested function calls and floating-point coercions.

## Feedback to T.A.s

1. **Bonus Difficulty Scaling:**
   It would be helpful to explicitly mention the relative difficulty of the bonus parts in the README. In my experience, the implementation complexity followed this order: `boolean` << `string` < `real` (easy to hard). Knowing this earlier would help students prioritize their time when it is limited.

2. **Spec Clarity:**
   The RISC-V tutorial and the provided stack machine examples in the `README.md` were very helpful for getting started quickly. The fixed 128-byte stack frame simplified the local variable management significantly.
