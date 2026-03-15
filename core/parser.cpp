#include "parser.h"
#include <sstream>
#include <regex>
#include <algorithm>

// Таблица соответствия: русский -> английский
const std::unordered_map<std::string, std::string> InstructionParser::RU_TO_EN = {
    {"сложи", "add"}, {"вычти", "sub"}, {"и", "and"}, {"или", "or"},
    {"исключи_или", "xor"}, {"сдвинь_влево", "sll"}, {"сдвинь_вправо", "srl"},
    {"загрузи", "lw"}, {"сохрани", "sw"}, {"загрузи_байт", "lb"},
    {"ветви_если_равно", "beq"}, {"ветви_если_не_равно", "bne"},
    {"ветви_если_меньше", "blt"}, {"перейти", "jal"}, {"перейти_рег", "jalr"},
    {"системный", "ecall"}, {"nop", "nop"}
};

// ✅ Глобальная таблица регистров (доступна из всех функций)
const std::unordered_map<std::string, int> InstructionParser::REG_ALIASES = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4},
    {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8}, {"s1", 9},
    {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14},
    {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19},
    {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24},
    {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}
};

std::string InstructionParser::normalizeRegister(const std::string& reg) {
    // Если уже числовой формат (x5)
    if (reg.size() > 1 && reg[0] == 'x') {
        try { 
            int num = std::stoi(reg.substr(1));
            if (num < 0 || num > 31) return "-1";  // ✅ Проверка границ
            return std::to_string(num); 
        }
        catch(...) { return "-1"; }
    }
    
    auto it = REG_ALIASES.find(reg);  // ✅ Используем глобальную таблицу
    return (it != REG_ALIASES.end()) ? std::to_string(it->second) : "-1";
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
    ParsedInstr result;
    
    // ✅ Trim пробелов в начале и конце
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    if (!trimmed.empty()) {
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
    }
    
    if (trimmed.empty() || trimmed.substr(0, 2) == "//" || trimmed[0] == '#' || trimmed[0] == '/') {
        result.is_valid = true;  // Пустая строка или комментарий — ок
        return result;
    }
    
    std::string cleaned = translateToEnglish(line, mode);
    
    // Простой парсер для формата: <opcode> <args>
    // Поддерживаем: add rd, rs1, rs2 | lw rd, offset(rs1) | beq rs1, rs2, label
    std::regex instr_re(R"((\w+)\s+([^,\s]+)\s*,\s*([^\s,]+)(?:\s*,\s*([^\s,]+))?)");
    std::regex load_re(R"((\w+)\s*([-\w]+)\s*,\s*\(([^)]+)\))");
    
    std::smatch match;
    
    if (std::regex_match(cleaned, match, load_re)) {
        // lw t0, 4(t1) -> opcode=lw, rd=t0, imm=4, rs1=t1
        result.opcode = match[1];
        result.rd = std::stoi(normalizeRegister(match[2]));
        result.rs1 = std::stoi(normalizeRegister(match[3]));
        
        // ✅ Проверка границ регистров
        if (result.rd < 0 || result.rd > 31 || result.rs1 < 0 || result.rs1 > 31) {
            result.is_valid = false;
            result.error_msg = "Неверный номер регистра (должен быть 0-31)";
            return result;
        }
        
        result.is_valid = true;
        return result;
    }
    
    if (std::regex_match(cleaned, match, instr_re)) {
        result.opcode = match[1];
        result.rd = std::stoi(normalizeRegister(match[2]));
        result.rs1 = std::stoi(normalizeRegister(match[3]));
        
        if (match[4].matched) {
            // Третий операнд: регистр или константа
            std::string third = match[4];
            if (third[0] == 'x' || REG_ALIASES.count(third)) {  // ✅ Используем глобальную таблицу
                result.rs2 = std::stoi(normalizeRegister(third));
            } else {
                // Константа
                try { result.rs2 = std::stoi(third); }
                catch(...) { result.rs2 = 0; }
            }
        }
        
        // ✅ Проверка границ регистров
        if (result.rd < 0 || result.rd > 31 || result.rs1 < 0 || result.rs1 > 31) {
            result.is_valid = false;
            result.error_msg = "Неверный номер регистра (должен быть 0-31)";
            return result;
        }
        
        result.is_valid = true;
        return result;
    }
    
    result.error_msg = "Не распознана инструкция: " + line;
    return result;
}
