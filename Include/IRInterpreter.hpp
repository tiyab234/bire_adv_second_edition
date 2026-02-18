/*
    Copyright (c) 2026 - Yann BOYER
*/
#ifndef IR_INTERPRETER_HPP
#define IR_INTERPRETER_HPP

#include "IRInstructions.hpp"
#include <array>
#include <vector>

class IRInterpreter {
  public:
    IRInterpreter();

    void LoadIRCode(const std::vector<IRInst> irPrg);
    void InterpretIRCode();

  private:
    static constexpr uint32_t TOTAL_MEM_CELLS = 100000;

    std::size_t m_instPtr;
    std::size_t m_memPtr;
    std::array<uint8_t, TOTAL_MEM_CELLS> m_mem;
    std::vector<IRInst> m_irPrg;
};

#endif
