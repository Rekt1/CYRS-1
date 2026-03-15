#ifndef PARSER_H
#define PARSER_H

#include "riscv_core.h"
#include <unordered_map>
#include <string>

class InstructionParser {
private:
    static const std::unordered_map<std::string, std::string> RU_TO_EN;
    static const std::unordered_map<std::string, int> REG_ALIASES;
    
    static std::string normalizeRegister(const std::string& reg);
    static std::string translateToEnglish(const std::string& instr, SyntaxMode mode);
    
public:
    // Парсит строку вида "сложи t0, t1, t2" или "add x1, x2, x3"
    // Возвращает: {opcode, rd, rs1, rs2, imm, is_valid}
    struct ParsedInstr {
        std::string opcode;
        int rd, rs1, rs2;
        int32_t imm;
        bool is_valid;
        std::string error_msg;
        
        ParsedInstr() : opcode(""), rd(-1), rs1(-1), rs2(-1), imm(0), is_valid(false), error_msg("") {}
    };
    
    static ParsedInstr parse(const std::string& line, SyntaxMode mode);
};

#endif
