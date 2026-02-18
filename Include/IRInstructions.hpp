/*
    Copyright (c) 2026 - Yann BOYER
*/
#ifndef IR_INSTRUCTIONS_HPP
#define IR_INSTRUCTIONS_HPP

#include <cstdint>
#include <optional>
#include <vector>

// SZe and MultMv are not mapped to any Brainfuck command !
// Other IR Instructions are direct mappings to their corresponding Brainfuck command !
enum class IRInstKind {
    IncrPtr = '>',
    DecrPtr = '<',
    IncrByte = '+',
    DecrByte = '-',
    ByteStdout = '.',
    ByteStdin = ',',
    JmpZe = '[',
    JmpNze = ']',
    SZe = 'z', // Mapped to [-] pattern !
    MultMv = 'm',
};

struct MoveDelta {
    int32_t offset;
    int32_t multiplier;
};

struct IRInst {
    IRInstKind kind;
    std::optional<uint32_t> operand;
    std::vector<MoveDelta> multiDeltas;
};

#endif
