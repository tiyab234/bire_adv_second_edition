/*
    Copyright (c) 2026 - Yann BOYER
*/
#include "IRGenerator.hpp"
#include <map>
#include <stdexcept>

void IRGenerator::AddIRInst(IRInst newInst) {
    if (m_tmpIRPrg.empty()) {
        m_tmpIRPrg.push_back(newInst);
        return;
    }

    IRInst &last = m_tmpIRPrg.back();

    if (newInst.kind == IRInstKind::SZe && (last.kind == IRInstKind::IncrByte || last.kind == IRInstKind::DecrByte)) {
        last = newInst;
        return;
    }
    m_tmpIRPrg.push_back(newInst);
}

void IRGenerator::ComputeJumpTargets() {
    std::vector<std::size_t> stack;
    std::size_t prgLen = m_tmpIRPrg.size();

    for (std::size_t i = 0; i < prgLen; i++) {
        if (m_tmpIRPrg[i].kind == IRInstKind::JmpZe) {
            stack.push_back(i);
        } else if (m_tmpIRPrg[i].kind == IRInstKind::JmpNze) {
            if (stack.empty()) {
                throw std::runtime_error("[FATAL ERROR] Unmatched ']' at byte " + std::to_string(i + 1));
            }
            std::size_t j = stack.back();
            stack.pop_back();

            m_tmpIRPrg[i].operand = j;
            m_tmpIRPrg[j].operand = i;
        }
    }

    if (!stack.empty()) throw std::runtime_error("[FATAL ERROR] Unmatched '[' at byte " + std::to_string(stack.back() + 1));
}

std::vector<IRInst> IRGenerator::GenerateIRFromRawInsts(const std::string &code) {
    m_lexer.Fill(code);

    char c = m_lexer.Next();
    while (c) {
        IRInst irInst;
        bool wasOptimized = false;

        if (c == '[') {
            int innerPos = 0;
            int currentOffset = 0;
            std::map<int, int> mvs;
            bool isLinear = true;

            while (m_lexer.Peek(innerPos) && m_lexer.Peek(innerPos) != ']') {
                char innerC = m_lexer.Peek(innerPos);
                if (innerC == '>')
                    currentOffset++;
                else if (innerC == '<')
                    currentOffset--;
                else if (innerC == '+')
                    mvs[currentOffset]++;
                else if (innerC == '-')
                    mvs[currentOffset]--;
                else {
                    isLinear = false;
                    break;
                }
                innerPos++;
            }

            if (isLinear && m_lexer.Peek(innerPos) == ']' && currentOffset == 0 && mvs[0] == -1) {
                // Ah... I'm dumb... I was doing a bit of stupid shit and the IR Generator completely forgot about
                // the existence of SZe, now it's fixed !
                // Tho, performance impact is meehhhhh, negligible !
                if (mvs.size() == 1) {
                    irInst.kind = IRInstKind::SZe;
                } else {
                    irInst.kind = IRInstKind::MultMv;
                    for (auto const &[off, mult] : mvs) {
                        if (off != 0) irInst.multiDeltas.push_back({off, mult});
                    }
                }
                for (int i = 0; i <= innerPos; ++i)
                    m_lexer.Next();
                wasOptimized = true;
            }
        }

        if (wasOptimized) {
            AddIRInst(irInst);
            c = m_lexer.Next();
            continue;
        }

        switch (c) {
            case '>':
            case '<':
            case '+':
            case '-':
            case '.':
            {
                uint32_t iCombo = 1;

                while (m_lexer.Peek(0) == c) {
                    iCombo++;
                    m_lexer.Next();
                }
                irInst = {static_cast<IRInstKind>(c), iCombo};
                AddIRInst(irInst);
            } break;
            case ',':
            case '[':
            case ']':
                irInst = {static_cast<IRInstKind>(c), {}};
                AddIRInst(irInst);
                break;
        }
        c = m_lexer.Next();
    }

    if (m_tmpIRPrg.empty()) throw std::runtime_error("[FATAL ERROR] This is not a valid Brainfuck program !");

    ComputeJumpTargets();
    return m_tmpIRPrg;
}
