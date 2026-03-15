// Для отладки ядра отдельно от Flask
int main(int argc, char* argv[]) {
    RISCVEmulator emu;
    emu.setMode(SyntaxMode::RUSSIAN);
    
    std::vector<std::string> test_code = {
        "// Тест на русском",
        "сложи x1, x2, x3",
        "сохрани x1, 0(x2)",
        "загрузи x4, 0(x2)",
        "ecall"
    };
    
    emu.loadProgram(test_code);
    
    while (true) {
        auto res = emu.step();
        if (!res.success) break;
        printf("PC=%08X | x1=%d | x2=%d | x3=%d\n", 
               res.state.pc, res.state.regs[1], res.state.regs[2], res.state.regs[3]);
    }
    return 0;
}
