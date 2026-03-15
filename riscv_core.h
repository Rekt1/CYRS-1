#ifndef RISCV_CORE_H
#define RISCV_CORE_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

// RV32I: 32 регистра по 32 бита
using RegFile = std::array<uint32_t, 32>;
using Memory = std::vector<uint8_t>;

struct CPUState {
    RegFile regs;           // x0-x31 (x0 всегда 0)
    uint32_t pc;            // Program Counter
    uint32_t memory_base;   // База памяти (по умолчанию 0x0)
    bool halted;            // Флаг остановки
};

struct ExecutionResult {
    bool success;
    std::string error;
    CPUState state;
    std::string output_log;
    int executed_line;      // Номер выполненной строки для подсветки
};

// Парсер поддерживает два режима
enum class SyntaxMode { ENGLISH, RUSSIAN };

#endif
