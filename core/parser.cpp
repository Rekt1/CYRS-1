#include "parser.h"
#include <sstream>
#include <regex>
#include <algorithm>  // Для trim

// ✅ Глобальная таблица регистров (доступна везде)
static const std::unordered_map<std::string, int> REG_ALIASES = {
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

InstructionParser::ParsedInstr InstructionParser::parse(const std::string& line, SyntaxMode mode) {
    ParsedInstr result = {"", -1, -1, -1, 0, false, ""};
    
    // ✅ Trim пробелов
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
    
    if (trimmed.empty() || trimmed.substr(0, 2) == "//" || trimmed[0] == '#') {
        result.is_valid = true;
        return result;
    }
    
    // ... остальной код с проверкой границ регистров
    if (result.rd < 0 || result.rd > 31 || result.rs1 < 0 || result.rs1 > 31) {
        result.is_valid = false;
        result.error_msg = "Неверный номер регистра (должен быть 0-31)";
        return result;
    }
    
    return result;
}
