#include "riscv_core.h"
#include "parser.h"
#include <sstream>
#include <iomanip>
#include <iostream>

// Для JSON парсинга (header-only библиотека)
// Скачать: https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
#include "json.hpp"
using json = nlohmann::json;

class RISCVEmulator {
private:
    CPUState cpu;
    Memory memory;
    SyntaxMode mode;
    std::vector<std::string> program_lines;
    int current_line;
    
    // Выполнение одной инструкции
    bool executeInstruction(const std::string& line) {
        auto parsed = InstructionParser::parse(line, mode);
        if (!parsed.is_valid) return false;
        if (parsed.opcode.empty()) return true; // Комментарий/пусто
        
        // ✅ Раздельные функции чтения и записи (x0 нельзя изменить!)
        auto R = [&](int idx) -> uint32_t {
            return (idx < 0 || idx > 31 || idx == 0) ? 0 : cpu.regs[idx];
        };
        
        auto W = [&](int idx, uint32_t val) {
            if (idx > 0 && idx <= 31) cpu.regs[idx] = val;  // ✅ x0 всегда 0
        };
        
        // === АРИФМЕТИЧЕСКИЕ ИНСТРУКЦИИ ===
        if (parsed.opcode == "add") {
            W(parsed.rd, R(parsed.rs1) + R(parsed.rs2));
        }
        else if (parsed.opcode == "sub") {
            W(parsed.rd, R(parsed.rs1) - R(parsed.rs2));
        }
        else if (parsed.opcode == "and") {
            W(parsed.rd, R(parsed.rs1) & R(parsed.rs2));
        }
        else if (parsed.opcode == "or") {
            W(parsed.rd, R(parsed.rs1) | R(parsed.rs2));
        }
        else if (parsed.opcode == "xor") {
            W(parsed.rd, R(parsed.rs1) ^ R(parsed.rs2));
        }
        else if (parsed.opcode == "sll") {
            W(parsed.rd, R(parsed.rs1) << (R(parsed.rs2) & 0x1F));
        }
        else if (parsed.opcode == "srl") {
            W(parsed.rd, R(parsed.rs1) >> (R(parsed.rs2) & 0x1F));
        }
        // === ЗАГРУЗКА/СОХРАНЕНИЕ ===
        else if (parsed.opcode == "lw") {
            uint32_t addr = R(parsed.rs1) + parsed.rs2;  // rs2 здесь = imm
            if (addr + 3 >= memory.size()) return false;
            // Little-endian загрузка 4 байт
            W(parsed.rd, memory[addr] | (memory[addr+1] << 8) |
                  (memory[addr+2] << 16) | (memory[addr+3] << 24));
        }
        else if (parsed.opcode == "sw") {
            uint32_t addr = R(parsed.rs1) + parsed.rs2;
            if (addr + 3 >= memory.size()) return false;
            memory[addr] = R(parsed.rd) & 0xFF;
            memory[addr+1] = (R(parsed.rd) >> 8) & 0xFF;
            memory[addr+2] = (R(parsed.rd) >> 16) & 0xFF;
            memory[addr+3] = (R(parsed.rd) >> 24) & 0xFF;
        }
        // === ВЕТВЛЕНИЯ (упрощённо) ===
        else if (parsed.opcode == "beq") {
            if (R(parsed.rs1) == R(parsed.rs2)) {
                // Для демо: просто пропускаем следующую инструкцию
                current_line++;
            }
        }
        else if (parsed.opcode == "bne") {
            if (R(parsed.rs1) != R(parsed.rs2)) {
                current_line++;
            }
        }
        else if (parsed.opcode == "blt") {
            if ((int32_t)R(parsed.rs1) < (int32_t)R(parsed.rs2)) {
                current_line++;
            }
        }
        // === ПЕРЕХОДЫ ===
        else if (parsed.opcode == "jal") {
            W(parsed.rd, cpu.pc + 4);  // Сохраняем адрес возврата
            // Для демо: просто переходим вперёд
        }
        // === ОСТАНОВКА ===
        else if (parsed.opcode == "ecall" || parsed.opcode == "nop") {
            if (parsed.opcode == "ecall") cpu.halted = true;
            cpu.pc += 4;
            return true;
        }
        else {
            return false; // Неизвестная инструкция
        }
        
        cpu.pc += 4;  // Стандартный шаг для RV32I
        return true;
    }
    
public:
    RISCVEmulator() : mode(SyntaxMode::ENGLISH), current_line(0) {
        cpu.regs.fill(0);
        cpu.pc = 0;
        cpu.memory_base = 0;
        cpu.halted = false;
        memory.resize(64 * 1024, 0);  // 64 КБ памяти
    }
    
    void setMode(SyntaxMode m) { mode = m; }
    
    void loadProgram(const std::vector<std::string>& lines) {
        program_lines = lines;
        current_line = 0;
        cpu.regs.fill(0);
        cpu.pc = 0;
        cpu.halted = false;
    }
    
    ExecutionResult step() {
        ExecutionResult res;
        res.state = cpu;
        res.executed_line = current_line;
        
        if (cpu.halted || current_line >= program_lines.size()) {
            res.success = false;
            res.error = cpu.halted ? "Процессор остановлен" : "Конец программы";
            return res;
        }
        
        bool ok = executeInstruction(program_lines[current_line]);
        if (!ok) {
            res.success = false;
            res.error = "Ошибка в строке " + std::to_string(current_line + 1);
            return res;
        }
        
        current_line++;
        res.state = cpu;
        return res;
    }
    
    ExecutionResult runAll() {
        ExecutionResult res;
        while (!cpu.halted && current_line < program_lines.size()) {
            res = step();
            if (!res.success) break;
        }
        return res;
    }
    
    void reset() {
        cpu.regs.fill(0);
        cpu.pc = 0;
        cpu.halted = false;
        current_line = 0;
    }
    
    const CPUState& getState() const { return cpu; }
    const Memory& getMemory() const { return memory; }
};

// ✅ ТОЧКА ВХОДА для коммуникации с Python
int main(int argc, char* argv[]) {
    RISCVEmulator emu;
    
    // Читаем JSON из stdin
    json input;
    try {
        std::cin >> input;
    } catch (const std::exception& e) {
        json error;
        error["success"] = false;
        error["error"] = std::string("Ошибка парсинга JSON: ") + e.what();
        std::cout << error.dump() << std::endl;
        return 1;
    }
    
    std::vector<std::string> lines = input["lines"].get<std::vector<std::string>>();
    std::string mode_str = input["mode"].get<std::string>();
    std::string action = input["action"].get<std::string>();
    
    SyntaxMode mode = (mode_str == "russian") ? SyntaxMode::RUSSIAN : SyntaxMode::ENGLISH;
    emu.setMode(mode);
    emu.loadProgram(lines);
    
    ExecutionResult result;
    if (action == "step") {
        result = emu.step();
    } else if (action == "run") {
        result = emu.runAll();
    } else if (action == "reset") {
        emu.reset();
        result.success = true;
        result.state = emu.getState();
        result.executed_line = 0;
    } else {
        result.success = false;
        result.error = "Неизвестное действие: " + action;
    }
    
    // Вывод JSON
    json output;
    output["success"] = result.success;
    output["error"] = result.error;
    output["pc"] = result.state.pc;
    
    json regs;
    for (int i = 0; i < 32; i++) {
        regs[std::to_string(i)] = result.state.regs[i];
    }
    output["registers"] = regs;
    
    // Память (первые 256 байт для экономии трафика)
    const Memory& mem = emu.getMemory();
    json mem_json;
    for (size_t i = 0; i < std::min(mem.size(), (size_t)256); i++) {
        mem_json[std::to_string(i)] = mem[i];
    }
    output["memory"] = mem_json;
    
    output["line"] = result.executed_line;
    output["output"] = result.output_log;
    
    std::cout << output.dump() << std::endl;
    
    return result.success ? 0 : 1;
}
