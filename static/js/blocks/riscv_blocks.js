/**
 * Определение блоков для RISC-V в Blockly.
 * Минимальный набор для демо.
 */
const RISCV_BLOCKS = [
    {
        "type": "riscv_arith",
        "message0": "%1 %2, %3, %4",
        "args0": [
            {"type": "field_dropdown", "name": "OP", "options": [
                ["сложи / add", "add"], ["вычти / sub", "sub"],
                ["и / and", "and"], ["или / or", "or"]
            ]},
            {"type": "field_input", "name": "RD", "text": "x1"},
            {"type": "field_input", "name": "RS1", "text": "x2"},
            {"type": "field_input", "name": "RS2", "text": "x3"}
        ],
        "previousStatement": null,
        "nextStatement": null,
        "colour": 230,
        "tooltip": "Арифметическая операция",
        "helpUrl": ""
    },
    {
        "type": "riscv_load",
        "message0": "загрузи %1, %2(%3)",
        "args0": [
            {"type": "field_input", "name": "RD", "text": "x1"},
            {"type": "field_number", "name": "OFFSET", "value": 0},
            {"type": "field_input", "name": "RS1", "text": "x2"}
        ],
        "previousStatement": null,
        "nextStatement": null,
        "colour": 190,
        "tooltip": "Загрузка слова из памяти"
    },
    {
        "type": "riscv_store",
        "message0": "сохрани %1, %2(%3)",
        "args0": [
            {"type": "field_input", "name": "RS2", "text": "x1"},
            {"type": "field_number", "name": "OFFSET", "value": 0},
            {"type": "field_input", "name": "RS1", "text": "x2"}
        ],
        "previousStatement": null,
        "nextStatement": null,
        "colour": 190,
        "tooltip": "Сохранение слова в память"
    },
    {
        "type": "riscv_label",
        "message0": "метка: %1",
        "args0": [{"type": "field_input", "name": "NAME", "text": "start"}],
        "previousStatement": null,
        "nextStatement": null,
        "colour": 330,
        "tooltip": "Метка для переходов"
    }
];

function getBlockDefinitions() {
    return RISCV_BLOCKS;
}

function getToolboxDefinition() {
    return {
        "kind": "flyoutToolbox",
        "contents": [
            {"kind": "block", "type": "riscv_arith"},
            {"kind": "block", "type": "riscv_load"},
            {"kind": "block", "type": "riscv_store"},
            {"kind": "block", "type": "riscv_label"},
            {"kind": "sep"},
            {"kind": "block", "type": "controls_if"},
            {"kind": "block", "type": "logic_compare"}
        ]
    };
}
