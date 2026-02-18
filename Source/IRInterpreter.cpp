/*
    Copyright (c) 2026 - Yann BOYER
*/
#include "IRInterpreter.hpp"
#include <iostream>
#include <print>

IRInterpreter::IRInterpreter() {
    m_instPtr = 0;
    m_memPtr = 0;
    m_mem.fill(0);
    m_irPrg.resize(0);
}

void IRInterpreter::LoadIRCode(const std::vector<IRInst> irPrg) { m_irPrg = irPrg; }

// NOTE : CURRENTLY THE IR INTERPRETER IS COMPLETELY BROKEN DUE TO CHANGE IN THE IR !
// NOTE : MAYBE I WILL FIX THIS, MAYBE !
void IRInterpreter::InterpretIRCode() {
    std::size_t prgLen = m_irPrg.size();

    // Finally, the most interesting part, IR Code execution.
    while (m_instPtr < prgLen) {
        IRInst irInst = m_irPrg[m_instPtr];

        switch (irInst.kind) {
            case IRInstKind::IncrPtr:
                m_memPtr += irInst.operand.value();
                break;
            case IRInstKind::DecrPtr:
                m_memPtr -= irInst.operand.value();
                break;
            case IRInstKind::IncrByte:
                m_mem[m_memPtr] += irInst.operand.value();
                break;
            case IRInstKind::DecrByte:
                m_mem[m_memPtr] -= irInst.operand.value();
                break;
            case IRInstKind::ByteStdout:
            {
                uint8_t repeatCount = irInst.operand.value();
                char byteToPrint = static_cast<char>(m_mem[m_memPtr]);

                for (uint8_t i = 0; i < repeatCount; i++) {
                    std::print("{}", byteToPrint);
                    std::fflush(stdout);
                }
            } break;
            case IRInstKind::ByteStdin:
            {
                char inputByte;
                std::cin >> inputByte;
                std::cin.clear();
                m_mem[m_memPtr] = static_cast<uint8_t>(inputByte);
            } break;
            case IRInstKind::JmpZe:
                if (m_mem[m_memPtr] == 0) m_instPtr = irInst.operand.value() - 1;
                break;
            case IRInstKind::JmpNze:
                if (m_mem[m_memPtr] != 0) m_instPtr = irInst.operand.value() - 1;
                break;
            case IRInstKind::SZe:
                m_mem[m_memPtr] = 0;
                break;
            case IRInstKind::MultMv:
                std::println("lol");
                break;
        }

        m_instPtr++;
    }
}
