#include "parser.h"
#include <sstream>
#include <regex>

// Таблица соответствия: русский -> английский
const std::unordered_map<std::string, std::string> InstructionParser::RU_TO_EN = {
    {"сложи", "add"}, {"вычти", "sub"}, {"и", "and"}, {"или", "or"},
    {"исключи_или", "xor"}, {"сдвинь_влево", "sll"}, {"сдвинь_вправо", "srl"},
    {"загрузи", "lw"}, {"сохрани", "sw"}, {"загрузи_байт", "lb"},
    {"ветви_если_равно", "beq"}, {"ветви_если_не_равно", "bne"},
    {"ветви_если_меньше", "blt"}, {"перейти", "jal"}, {"перейти_рег", "jalr"},
    {"системный", "ecall"}, {"nop", "nop"}
};

std::string InstructionParser::normalizeRegister(const std::string& reg) {
    // Поддерживаем: x0-x31, zero, ra, sp, gp, tp, t0-t6, s0-s11, a0-a7
    static std::unordered_map<std::string, int> aliases = {
        {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4},
        {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8}, {"s1", 9},
        {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14},
        {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19},
        {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24},
        {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
        {"t5", 30}, {"t6", 31}
    };
    
    // Если уже числовой формат (x5)
    if (reg.size() > 1 && reg[0] == 'x') {
        try { return std::to_string(std::stoi(reg.substr(1))); } 
        catch(...) { return "-1"; }
    }
    
    auto it = aliases.find(reg);
    return (it != aliases.end()) ? std::to_string(it->second) : "-1";
}

std::string InstructionParser::translateToEnglish(const std::string& instr, SyntaxMode mode) {
    if (mode == SyntaxMode::ENGLISH) return instr;
    
    std::string result = instr;
    // Удаляем комментарии
    size_t comment_pos = result.find("//");
    if (comment_pos != std::string::npos) result = result.substr(0, comment_pos);
    
    // Извлекаем мнемонику (первое слово)
    std::istringstream iss(result);
    std::string mnemonic;
    iss >> mnemonic;
    
    // Переводим если есть в таблице
    auto it = RU_TO_EN.find(mnemonic);
    if (it != RU_TO_EN.end()) {
        result.replace(0, mnemonic.size(), it->second);
    }
    return result;
}

InstructionParser::ParsedInstr InstructionParser::parse(const std::string& line, SyntaxMode mode) {
    ParsedInstr result = {"", -1, -1, -1, 0, false, ""};
    
    if (line.empty() || line[0] == '/' || line[0] == '#') {
        result.is_valid = true; // Пустая строка или комментарий — ок
        return result;
    }
    
    std::string cleaned = translateToEnglish(line, mode);
    
    // Простой парсер для формата: <opcode> <args>
    // Поддерживаем: add rd, rs1, rs2 | lw rd, offset(rs1) | beq rs1, rs2, label
    std::regex instr_re(R"((\w+)\s+([^,\s]+)\s*,\s*([^\s,]+)(?:\s*,\s*([^\s,]+))?)");
    std::regex load_re(R"((\w+)\s+([-\w]+)\s*,\s*\(([^)]+)\))");
    
    std::smatch match;
    
    if (std::regex_match(cleaned, match, load_re)) {
        // lw t0, 4(t1) -> opcode=lw, rd=t0, imm=4, rs1=t1
        result.opcode = match[1];
        result.rd = std::stoi(normalizeRegister(match[2]));
        result.rs1 = std::stoi(normalizeRegister(match[3]));
        result.is_valid = (result.rd >= 0 && result.rs1 >= 0);
        if (!result.is_valid) result.error_msg = "Неверный регистр";
        return result;
    }
    
    if (std::regex_match(cleaned, match, instr_re)) {
        result.opcode = match[1];
        result.rd = std::stoi(normalizeRegister(match[2]));
        result.rs1 = std::stoi(normalizeRegister(match[3]));
        
        if (match[4].matched) {
            // Третий операнд: регистр или константа
            std::string third = match[4];
            if (third[0] == 'x' || aliases.count(third)) {
                result.rs2 = std::stoi(normalizeRegister(third));
            } else {
                // Константа
                try { result.rs2 = std::stoi(third); } 
                catch(...) { result.rs2 = 0; }
            }
        }
        result.is_valid = (result.rd >= 0 && result.rs1 >= 0);
        if (!result.is_valid) result.error_msg = "Неверный регистр";
        return result;
    }
    
    result.error_msg = "Не распознана инструкция: " + line;
    return result;
}
