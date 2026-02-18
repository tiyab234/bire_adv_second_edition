/*
    Copyright (c) 2026 - Yann BOYER
 */
#include "IRJit.hpp"
#include <cstdint>
#include <cstdlib>
#include <map>
#include <print>
#include <pthread.h>
#include <stdexcept>
#include <sys/mman.h>

IRJit::IRJit() {
    m_capacity = DEFAULT_JIT_CAPACITY;
    m_offset = 0;
    // AHAHAHAHAHH
    // Fun part(lol no...) ?
    // Here you can see PROT_EXEC.
    // Without it we get Bus Error !
    // Hmm, I remember saying a brain was too expensive... :(
    m_jBuf = mmap(nullptr, m_capacity, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);

    if (m_jBuf == MAP_FAILED) throw std::bad_alloc();
}

IRJit::~IRJit() { munmap(m_jBuf, m_capacity); }

void IRJit::LoadIRCode(const std::vector<IRInst> irPrg) { m_irPrg = irPrg; }

uint32_t IRJit::EncodeARMInst(ARMInstT instType, uint8_t rd, uint8_t rn, uint32_t imm) {
    uint32_t armInst = 0;

    switch (instType) {
        case ARMInstT::AddImm:
            // ADD IMM - 64bit
            armInst = 0x91000000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::SubImm:
            // SUB IMM - 64bit
            armInst = 0xD1000000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::AddwImm:
            // ADD IMM - 32bit
            armInst = 0x11000000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::SubwImm:
            // SUB IMM - 32bit
            armInst = 0x51000000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::MAddw:
            // MADD - 32bit
            armInst = 0x1B000000;
            armInst |= ((imm & 0x1F) << 16);
            armInst |= ((rd & 0x1F) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Ldrb:
            // LDRB
            armInst = 0x39400000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Strb:
            // STRB
            armInst = 0x39000000;
            armInst |= ((imm & 0xFFF) << 10);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Ldurb:
            // LDURB
            armInst = 0x38400000;
            armInst |= ((imm & 0x1FF) << 12);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Sturb:
            // STURB
            armInst = 0x38000000;
            armInst |= ((imm & 0x1FF) << 12);
            armInst |= ((rn & 0x1F) << 5);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Movz:
            // MOVZ - 64bit
            armInst = 0xD2800000;
            armInst |= (((imm & 0xFFFF) << 5) & 0x001FFFE0);
            armInst |= (rd & 0x1F);
            break;
        case ARMInstT::Movk:
        {
            // MOVK - 64bit
            uint8_t hw = (imm >> 16) & 0x3;
            uint16_t imm16 = imm & 0xFFFF;
            armInst = 0xF2800000;
            armInst |= (hw << 21);
            armInst |= ((imm16 << 5) & 0x001FFFE0);
            armInst |= (rd & 0x1F);
        } break;
        case ARMInstT::Svc:
            // SVC
            armInst = 0xD4000001;
            armInst |= (((imm & 0xFFFF) << 5) & 0x001FFFE0);
            break;
        case ARMInstT::Ret:
            // RET
            armInst = 0xD65F03C0;
            break;
        case ARMInstT::Cbz:
        {
            int32_t offset = static_cast<int32_t>(imm);
            uint32_t offsetBits = static_cast<uint32_t>(offset) & 0x7FFFF;
            armInst = 0x34000000; // CBZ
            armInst |= (offsetBits << 5);
            armInst |= (rd & 0x1F);
        } break;
        case ARMInstT::Cbnz:
        {
            int32_t offset = static_cast<int32_t>(imm);
            uint32_t offsetBits = static_cast<uint32_t>(offset) & 0x7FFFF;
            armInst = 0x35000000; // CBNZ
            armInst |= (offsetBits << 5);
            armInst |= (rd & 0x1F);
        } break;
    }

    return armInst;
}

void IRJit::EmitInst(const uint32_t inst) {
    if (m_offset + sizeof(uint32_t) > m_capacity) throw std::runtime_error("[FATAL ERROR] JIT overflow !");

    uint32_t *p = reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(m_jBuf) + m_offset);
    *p = inst;
    m_offset += sizeof(uint32_t);
}

void IRJit::CompileToARMFromIR() {
    m_offset = 0;
    pthread_jit_write_protect_np(0); // So we can actually write our JIT-ed insts.

    uint64_t mPtr = reinterpret_cast<uint64_t>(m_jMem);

    // Load tape pointer into x0.
    EmitInst(EncodeARMInst(ARMInstT::Movz, 0, 0, mPtr & 0xFFFF));
    EmitInst(EncodeARMInst(ARMInstT::Movk, 0, 0, ((mPtr >> 16) & 0xFFFF) | (1 << 16)));
    EmitInst(EncodeARMInst(ARMInstT::Movk, 0, 0, ((mPtr >> 32) & 0xFFFF) | (2 << 16)));
    EmitInst(EncodeARMInst(ARMInstT::Movk, 0, 0, ((mPtr >> 48) & 0xFFFF) | (3 << 16)));

    int32_t vPOffset = 0;
    auto SyncPtr = [&]() {
        if (vPOffset != 0) {
            if (vPOffset > 0)
                EmitInst(EncodeARMInst(ARMInstT::AddImm, 0, 0, vPOffset));
            else
                EmitInst(EncodeARMInst(ARMInstT::SubImm, 0, 0, std::abs(vPOffset)));
            vPOffset = 0;
        }
    };

    // 1st Pass -> Calculate offsets with exact simulation.
    int32_t simVPOffset = 0;
    std::map<std::size_t, std::size_t> irToArmOffset;
    std::size_t tempOffset = m_offset;

    for (std::size_t i = 0; i < m_irPrg.size(); i++) {
        const IRInst &irInst = m_irPrg[i];

        // Simulation of SyncPtr().
        auto SimSync = [&]() {
            if (simVPOffset != 0) {
                tempOffset += 4; // We know an ADD/SUB will be emitted.
                simVPOffset = 0;
            }
        };

        switch (irInst.kind) {
            case IRInstKind::IncrPtr:
                simVPOffset += irInst.operand.value();
                break;
            case IRInstKind::DecrPtr:
                simVPOffset -= irInst.operand.value();
                break;
            case IRInstKind::IncrByte:
            case IRInstKind::DecrByte:
                SimSync();
                irToArmOffset[i] = tempOffset;
                tempOffset += 12;
                break;
            case IRInstKind::JmpZe:
            case IRInstKind::JmpNze:
                SimSync();
                irToArmOffset[i] = tempOffset;
                tempOffset += 8;
                break;
            case IRInstKind::SZe:
                SimSync();
                irToArmOffset[i] = tempOffset;
                tempOffset += 8;
                break;
            case IRInstKind::MultMv:
                SimSync();
                irToArmOffset[i] = tempOffset;
                tempOffset += (1 + (irInst.multiDeltas.size() * 4) + 2) * 4;
                break;
            case IRInstKind::ByteStdout:
            {
                SimSync();
                uint32_t repeatCount = irInst.operand.value();
                irToArmOffset[i] = tempOffset;
                tempOffset += (1 + repeatCount * 6 + 1) * 4;
            } break;
            case IRInstKind::ByteStdin:
                // Honestly, I'm too lazy to even bother with this instruction...
                // I mean, I implemented it for the interpreter but eh...
                // It's my project after all ! So fuck it !
                // (pun intented lol)
                throw std::runtime_error("[FATAL ERROR] ByteStdin not implemented !");
                break;
        }
    }

    // 2nd Pass -> We emit ARM instructions.
    for (const auto &irInst : m_irPrg) {

        switch (irInst.kind) {
            case IRInstKind::IncrPtr:
            {
                uint32_t operand = irInst.operand.value();
                vPOffset += operand;
            } break;
            case IRInstKind::DecrPtr:
            {
                uint32_t operand = irInst.operand.value();
                vPOffset -= operand;
            } break;
            case IRInstKind::IncrByte:
            case IRInstKind::DecrByte:
            {
                SyncPtr();

                uint32_t operand = irInst.operand.value();
                EmitInst(EncodeARMInst(ARMInstT::Ldrb, 1, 0, 0));
                if (irInst.kind == IRInstKind::IncrByte)
                    EmitInst(EncodeARMInst(ARMInstT::AddwImm, 1, 1, operand));
                else
                    EmitInst(EncodeARMInst(ARMInstT::SubwImm, 1, 1, operand));
                EmitInst(EncodeARMInst(ARMInstT::Strb, 1, 0, 0));
            } break;
            case IRInstKind::ByteStdout:
            {
                SyncPtr();
                uint32_t repeatCount = irInst.operand.value();

                EmitInst(EncodeARMInst(ARMInstT::AddImm, 9, 0, 0));

                for (uint32_t i = 0; i < repeatCount; i++) {
                    EmitInst(EncodeARMInst(ARMInstT::Movz, 0, 0, 1));
                    EmitInst(EncodeARMInst(ARMInstT::AddImm, 1, 9, 0));
                    EmitInst(EncodeARMInst(ARMInstT::Movz, 2, 0, 1));
                    EmitInst(EncodeARMInst(ARMInstT::Movz, 16, 0, 0x0004));
                    EmitInst(EncodeARMInst(ARMInstT::Movk, 16, 0, 0x0200 | (1 << 16)));
                    EmitInst(EncodeARMInst(ARMInstT::Svc, 0, 0, 0));
                }

                EmitInst(EncodeARMInst(ARMInstT::AddImm, 0, 9, 0));
            } break;
            case IRInstKind::ByteStdin:
                // Honestly, I'm too lazy to even bother with this instruction...
                // I mean, I implemented it for the interpreter but eh...
                // It's my project after all ! So fuck it !
                // (pun intented lol)
                throw std::runtime_error("[FATAL ERROR] ByteStdin not implemented !");
                break;
            case IRInstKind::JmpZe:
            {
                SyncPtr();
                EmitInst(EncodeARMInst(ARMInstT::Ldrb, 1, 0, 0));

                std::size_t targetIRIdx = irInst.operand.value();
                std::size_t currentArmOffset = m_offset;
                std::size_t targetArmOffset = irToArmOffset[targetIRIdx];

                int32_t offset = (static_cast<int32_t>(targetArmOffset) - static_cast<int32_t>(currentArmOffset)) / 4;

                // If offset does not fit in 19bits that's not good !
                // The range is : -262144 to 262143
                if (offset < -262144 || offset > 262143) throw std::runtime_error("Branch offset too large: " + std::to_string(offset));

                EmitInst(EncodeARMInst(ARMInstT::Cbz, 1, 0, offset));
            } break;
            case IRInstKind::JmpNze:
            {
                SyncPtr();
                EmitInst(EncodeARMInst(ARMInstT::Ldrb, 1, 0, 0));

                std::size_t targetIRIdx = irInst.operand.value();
                std::size_t currentArmOffset = m_offset;
                std::size_t targetArmOffset = irToArmOffset[targetIRIdx];

                int32_t offset = (static_cast<int32_t>(targetArmOffset) - static_cast<int32_t>(currentArmOffset)) / 4;

                // If offset does not fit in 19bits that's not good !
                // The range is : -262144 to 262143
                if (offset < -262144 || offset > 262143) throw std::runtime_error("Branch offset too large: " + std::to_string(offset));

                EmitInst(EncodeARMInst(ARMInstT::Cbnz, 1, 0, offset));
            } break;
            case IRInstKind::SZe:
                SyncPtr();
                EmitInst(EncodeARMInst(ARMInstT::Movz, 1, 0, 0));
                EmitInst(EncodeARMInst(ARMInstT::Strb, 1, 0, 0));
                break;
            case IRInstKind::MultMv:
            {
                SyncPtr();
                EmitInst(EncodeARMInst(ARMInstT::Ldrb, 10, 0, 0));

                for (const auto &delta : irInst.multiDeltas) {
                    EmitInst(EncodeARMInst(ARMInstT::Movz, 11, 0, static_cast<uint8_t>(delta.multiplier)));
                    EmitInst(EncodeARMInst(ARMInstT::Ldurb, 12, 0, delta.offset));
                    EmitInst(EncodeARMInst(ARMInstT::MAddw, 12, 10, 11));
                    EmitInst(EncodeARMInst(ARMInstT::Sturb, 12, 0, delta.offset));
                }

                EmitInst(EncodeARMInst(ARMInstT::Movz, 10, 0, 0));
                EmitInst(EncodeARMInst(ARMInstT::Strb, 10, 0, 0));
            } break;
        }
    }

    SyncPtr(); // We need this...

    // Final return.
    EmitInst(EncodeARMInst(ARMInstT::Ret));

    // We finished emiting JIT-ed ARM insts.
    // Now we need to re-enable protection.
    // Otherwise we won't be able to execute it.
    // JIT code is W^R, Write XOR Read.
    // Which means it can't be written(generated) and read(executed) at the same time.
    pthread_jit_write_protect_np(1);
}

void IRJit::ExecuteARM() {
    using Fn = void (*)();
    Fn fn = reinterpret_cast<Fn>(m_jBuf);
    fn();
}
