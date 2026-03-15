// core/main.cpp
// Точка входа для коммуникации с Python через JSON (stdin/stdout)

#include "riscv_core.h"
#include "parser.h"
#include <iostream>
#include <string>

// JSON библиотека (скачайте: https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp)
#include "json.hpp"
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    // Создаём эмулятор
    RISCVEmulator emu;
    
    // Читаем весь JSON из stdin
    json input;
    try {
        std::cin >> input;
    } catch (const std::exception& e) {
        // Ошибка парсинга входных данных
        json error;
        error["success"] = false;
        error["error"] = std::string("JSON parse error: ") + e.what();
        error["pc"] = 0;
        error["registers"] = json::object();
        error["memory"] = json::object();
        error["line"] = -1;
        error["output"] = "";
        std::cout << error.dump() << std::endl;
        return 1;
    }
    
    // Извлекаем параметры
    std::vector<std::string> lines;
    try {
        lines = input["lines"].get<std::vector<std::string>>();
    } catch (...) {
        lines = {};  // Пустая программа
    }
    
    std::string mode_str = input.value("mode", "english");
    std::string action = input.value("action", "step");
    
    // Устанавливаем режим синтаксиса
    SyntaxMode mode = (mode_str == "russian") ? SyntaxMode::RUSSIAN : SyntaxMode::ENGLISH;
    emu.setMode(mode);
    
    // Загружаем программу
    emu.loadProgram(lines);
    
    // Выполняем действие
    ExecutionResult result;
    
    if (action == "step") {
        result = emu.step();
    } 
    else if (action == "run") {
        result = emu.runAll();
    } 
    else if (action == "reset") {
        emu.reset();
        result.success = true;
        result.state = emu.getState();
        result.executed_line = 0;
        result.error = "";
    } 
    else {
        result.success = false;
        result.error = "Unknown action: " + action;
        result.state = emu.getState();
        result.executed_line = -1;
    }
    
    // Формируем ответ в JSON
    json output;
    output["success"] = result.success;
    output["error"] = result.error;
    output["pc"] = result.state.pc;
    
    // Регистры: { "0": 0, "1": 42, ... }
    json regs = json::object();
    for (int i = 0; i < 32; i++) {
        regs[std::to_string(i)] = result.state.regs[i];
    }
    output["registers"] = regs;
    
    // Память: только первые 256 байт (для экономии трафика)
    const Memory& mem = emu.getMemory();
    json mem_json = json::object();
    size_t mem_limit = std::min(mem.size(), (size_t)256);
    for (size_t i = 0; i < mem_limit; i++) {
        mem_json[std::to_string(i)] = mem[i];
    }
    output["memory"] = mem_json;
    
    output["line"] = result.executed_line;
    output["output"] = result.output_log;
    
    // Выводим результат и завершаем
    std::cout << output.dump() << std::endl;
    
    return result.success ? 0 : 1;
}
