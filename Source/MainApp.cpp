/*
    Copyright (c) 2026 - Yann BOYER
*/
#include "IRGenerator.hpp"
#include "IRInstructions.hpp"
#include "IRInterpreter.hpp"
#include "IRJit.hpp"
#include <cstdlib>
#include <exception>
#include <fstream>
#include <print>
#include <string>

enum class ProgramExecMode {
    JitMode = 1,
    InterpreterMode = 0,
};

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::println(stderr, "[FATAL ERROR] No Brainfuck program provided !");
        std::println(stdout, "[INFO] EXEC_MODE is optional ! It'll default to Interpreter !");
        std::println(stdout, "[INFO] EXEC_MODE available : INT | JIT ");
        std::println(stdout, "[INFO] Usage ./bire_adv EXEC_MODE program.bf");
        return EXIT_FAILURE;
    }

    ProgramExecMode execMode;
    unsigned int prgPathArgIdx;

    std::string modeStrEncoded = "INT"; // Default execution mode ! Why ? Portability !
    std::string prgPath;

    if (std::string(argv[1]) == "INT" || std::string(argv[1]) == "JIT") {
        if (argc < 3) {
            std::println("Missing file path !");
            return EXIT_FAILURE;
        }
        modeStrEncoded = std::string(argv[1]);
        prgPath = std::string(argv[2]);
    } else {
        prgPath = std::string(argv[1]);
    }

    if (modeStrEncoded == "INT")
        execMode = ProgramExecMode::InterpreterMode;
    else if (modeStrEncoded == "JIT")
        execMode = ProgramExecMode::JitMode;

    std::ifstream prgFile(prgPath);

    if (!prgFile.is_open()) {
        std::println(stderr, "[FATAL ERROR] Unable to open the file !");
        return EXIT_FAILURE;
    }

    char ch;
    std::string prgBuf;
    while (prgFile.good()) {
        prgFile.get(ch);
        prgBuf.push_back(ch);
    }

    prgFile.close();

    IRGenerator irGenerator;
    std::vector<IRInst> irPrg;

    try {
        irPrg = irGenerator.GenerateIRFromRawInsts(prgBuf);
    } catch (std::exception &e) {
        std::println("{}", e.what());
        return EXIT_FAILURE;
    }

    switch (execMode) {
        case ProgramExecMode::InterpreterMode:
        {
            // NOTE : CURRENTLY THE IR INTERPRETER IS COMPLETELY BROKEN DUE TO CHANGE IN THE IR !
            // NOTE : MAYBE I WILL FIX THIS, MAYBE !
            std::println(stderr, "[FATAL ERROR] IR Interpreter is currently broken due to change in the IR !");
            return EXIT_FAILURE;
            IRInterpreter irInterpreter;
            irInterpreter.LoadIRCode(irPrg);
            irInterpreter.InterpretIRCode();
        } break;
        case ProgramExecMode::JitMode:
        {
            try {
                IRJit irJit;
                irJit.LoadIRCode(irPrg);
                irJit.CompileToARMFromIR();
                irJit.ExecuteARM();
            } catch (std::exception &e) {
                std::println("{}", e.what());
                return EXIT_FAILURE;
            }
        } break;
    }

    return EXIT_SUCCESS;
}
