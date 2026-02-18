/*
    Copyright (c) 2026 - Yann BOYER
*/
#ifndef IR_GENERATOR_HPP
#define IR_GENERATOR_HPP

#include "IRInstructions.hpp"
#include "Lexer.hpp"
#include <string>
#include <vector>

class IRGenerator {
  public:
    IRGenerator() = default;

    std::vector<IRInst> GenerateIRFromRawInsts(const std::string &code);

  private:
    void AddIRInst(IRInst newInst);
    void ComputeJumpTargets();

    std::vector<IRInst> m_tmpIRPrg;
    Lexer m_lexer;
};

#endif
