#include "riscv_core.h"
#include "parser.h"
#include <sstream>
#include <iomanip>

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
        
        // Функции-помощники для доступа к регистрам
        const auto& R = [&](int idx) -> uint32_t& { 
            return (idx == 0) ? cpu.regs[0] : cpu.regs[idx]; 
        };
        
        // === АРИФМЕТИЧЕСКИЕ ИНСТРУКЦИИ ===
        if (parsed.opcode == "add") {
            R(parsed.rd) = R(parsed.rs1) + R(parsed.rs2);
        }
        else if (parsed.opcode == "sub") {
            R(parsed.rd) = R(parsed.rs1) - R(parsed.rs2);
        }
        else if (parsed.opcode == "and") {
            R(parsed.rd) = R(parsed.rs1) & R(parsed.rs2);
        }
        else if (parsed.opcode == "or") {
            R(parsed.rd) = R(parsed.rs1) | R(parsed.rs2);
        }
        else if (parsed.opcode == "xor") {
            R(parsed.rd) = R(parsed.rs1) ^ R(parsed.rs2);
        }
        // === ЗАГРУЗКА/СОХРАНЕНИЕ ===
        else if (parsed.opcode == "lw") {
            uint32_t addr = R(parsed.rs1) + parsed.rs2; // rs2 здесь = imm
            if (addr + 3 >= memory.size()) return false;
            // Little-endian загрузка 4 байт
            R(parsed.rd) = memory[addr] | (memory[addr+1] << 8) | 
                          (memory[addr+2] << 16) | (memory[addr+3] << 24);
        }
        else if (parsed.opcode == "sw") {
            uint32_t addr = R(parsed.rs1) + parsed.rs2;
            if (addr + 3 >= memory.size()) return false;
            memory[addr] = R(parsed.rd) & 0xFF;
            memory[addr+1] = (R(parsed.rd) >> 8) & 0xFF;
            memory[addr+2] = (R(parsed.rd) >> 16) & 0xFF;
            memory[addr+3] = (R(parsed.rd) >> 24) & 0xFF;
        }
        // === ВЕТВЛЕНИЯ ===
        else if (parsed.opcode == "beq") {
            if (R(parsed.rs1) == R(parsed.rs2)) {
                // Упрощённо: переход по метке (в реальном коде нужен map меток)
                // Для демо: просто увеличиваем PC
                return true;
            }
        }
        // === ОСТАНОВКА ===
        else if (parsed.opcode == "ecall" || parsed.opcode == "nop") {
            if (parsed.opcode == "ecall") cpu.halted = true;
            return true;
        }
        else {
            return false; // Неизвестная инструкция
        }
        
        cpu.pc += 4; // Стандартный шаг для RV32I
        return true;
    }
    
public:
    RISCVEmulator() : mode(SyntaxMode::ENGLISH), current_line(0) {
        cpu.regs.fill(0);
        cpu.pc = 0;
        cpu.halted = false;
        memory.resize(64 * 1024, 0); // 64 КБ памяти — достаточно для демо
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
        ExecutionResult res{true, "", cpu, "", current_line};
        
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
