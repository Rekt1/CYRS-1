/**
 * Генератор русского ассемблера из блоков Blockly.
 */
if (typeof Blockly !== 'undefined') {
    Blockly.RiscvRu = new Blockly.Generator('RISCV_RU');
    
    Blockly.RiscvRu.forBlock['riscv_arith'] = function(block) {
        const op = block.getFieldValue('OP');
        const rd = block.getFieldValue('RD');
        const rs1 = block.getFieldValue('RS1');
        const rs2 = block.getFieldValue('RS2');
        return `${op} ${rd}, ${rs1}, ${rs2}\n`;
    };
    
    Blockly.RiscvRu.forBlock['riscv_load'] = function(block) {
        const rd = block.getFieldValue('RD');
        const offset = block.getFieldValue('OFFSET');
        const rs1 = block.getFieldValue('RS1');
        return `загрузи ${rd}, ${offset}(${rs1})\n`;
    };
    
    Blockly.RiscvRu.forBlock['riscv_store'] = function(block) {
        const rs2 = block.getFieldValue('RS2');
        const offset = block.getFieldValue('OFFSET');
        const rs1 = block.getFieldValue('RS1');
        return `сохрани ${rs2}, ${offset}(${rs1})\n`;
    };
    
    Blockly.RiscvRu.forBlock['riscv_label'] = function(block) {
        const name = block.getFieldValue('NAME');
        return `${name}:\n`;
    };
    
    // Пустые заглушки для стандартных блоков (можно расширить)
    Blockly.RiscvRu.forBlock['controls_if'] = function() { return '// if\n'; };
    Blockly.RiscvRu.forBlock['logic_compare'] = function() { return '=='; };
}
