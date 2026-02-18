/*
    Copyright (c) 2026 - Yann BOYER
*/
#ifndef IR_JIT_HPP
#define IR_JIT_HPP

#include "IRInstructions.hpp"
#include <vector>

// This is shitty, but eh ?
// I'm not using more than that !
// The EncodeARMInst() function that uses this enum does not need more !
enum class ARMInstT {
    AddImm,  // ADD IMM - 64bit
    SubImm,  // SUB IMM - 64bit
    AddwImm, // ADD IMM - 32bit
    SubwImm, // SUB IMM - 32bit
    MAddw,   // Multiply-Add, 32bit
    Ldrb,    // LDRB
    Strb,    // STRB
    Ldurb,   // LDURB
    Sturb,   // STURB
    Movz,    // MOVZ - 64bit
    Movk,    // MOVK - 64bit
    Svc,     // SVC, for syscall
    Ret,     // Return
    Cbz,     // CBZ, Compare and Branch on Zero
    Cbnz,    // CBNZ, Compare and Branch on Non-Zero
};

class IRJit {
  public:
    IRJit();
    ~IRJit();

    void LoadIRCode(const std::vector<IRInst> irPrg);
    void CompileToARMFromIR();
    void ExecuteARM();

  private:
    static constexpr std::size_t DEFAULT_JIT_CAPACITY = 100000;
    // Same as TOTAL_MEM_CELLS from interpreter.
    static constexpr uint32_t TOTAL_MEM_CELLS_JIT = 100000;

    uint32_t EncodeARMInst(ARMInstT instType, uint8_t rd = 0, uint8_t rn = 0, uint32_t imm = 0);
    void EmitInst(const uint32_t inst);

    std::vector<IRInst> m_irPrg;
    void *m_jBuf;
    uint8_t m_jMem[TOTAL_MEM_CELLS_JIT] = {0};
    std::size_t m_capacity;
    std::size_t m_offset;
};

#endif
