#ifndef PARSER_H
#define PARSER_H

#include "riscv_core.h"
#include <unordered_map>

class InstructionParser {
private:
    static const std::unordered_map<std::string, std::string> RU_TO_EN;
    
    std::string normalizeRegister(const std::string& reg);
    std::string translateToEnglish(const std::string& instr, SyntaxMode mode);
    
public:
    // Парсит строку вида "сложи t0, t1, t2" или "add x1, x2, x3"
    // Возвращает: {opcode, rd, rs1, rs2, imm, is_valid}
    struct ParsedInstr {
        std::string opcode;
        int rd, rs1, rs2;
        int32_t imm;
        bool is_valid;
        std::string error_msg;
    };
    
    static ParsedInstr parse(const std::string& line, SyntaxMode mode);
};

#endif
