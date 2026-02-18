/*
    Copyright (c) 2026 - Yann BOYER
*/
#include "Lexer.hpp"

Lexer::Lexer() {
    m_codePos = 0;
    m_codeLen = 0;
    m_code.resize(0);
}

void Lexer::Fill(const std::string &code) {
    m_code.assign(code.begin(), code.end());
    m_codeLen = m_code.size();
}

bool Lexer::IsValidBFCmd(const char pInst) {
    const std::string VALID_CMDS = "><+-.,[]";
    if (VALID_CMDS.find(pInst) != std::string::npos)
        return true;
    else
        return false;
}

char Lexer::Next() {
    while (m_codePos < m_codeLen && !IsValidBFCmd(m_code[m_codePos]))
        m_codePos++;

    if (m_codePos >= m_codeLen) return 0; // EOF.
    return m_code[m_codePos++];
}

char Lexer::Peek(int offset) {
    if (m_codePos + offset >= m_codeLen) return '\0';
    return m_code[m_codePos + offset];
}
