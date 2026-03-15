/**
 * Основной контроллер приложения.
 * Минимум зависимостей, чистый JS.
 */
document.addEventListener('DOMContentLoaded', () => {
    // DOM элементы
    const els = {
        editor: document.getElementById('code-editor'),
        syntaxOverlay: document.getElementById('syntax-overlay'),
        modeSelect: document.getElementById('syntax-mode'),
        btnRun: document.getElementById('btn-run'),
        btnStep: document.getElementById('btn-step'),
        btnReset: document.getElementById('btn-reset'),
        btnVisual: document.getElementById('btn-visual-mode'),
        visualOverlay: document.getElementById('visual-mode-overlay'),
        btnApplyBlocks: document.getElementById('btn-apply-blocks'),
        btnCloseVisual: document.getElementById('btn-close-visual'),
        pcValue: document.getElementById('pc-value'),
        registersGrid: document.querySelector('.registers-grid'),
        memoryView: document.getElementById('memory-view'),
        outputLog: document.getElementById('output-log'),
        helpList: document.getElementById('help-list'),
        tabs: document.querySelectorAll('.tab-btn'),
        tabContents: document.querySelectorAll('.tab-content')
    };
    
    // Состояние
    let currentMode = 'russian';
    let blocklyWorkspace = null;
    
    // === Инициализация ===
    
    // Подсветка синтаксиса
    SyntaxHighlighter.bind(els.editor, els.syntaxOverlay, currentMode);
    
    // Переключение режима синтаксиса
    els.modeSelect.addEventListener('change', (e) => {
        currentMode = e.target.value;
        SyntaxHighlighter.bind(els.editor, els.syntaxOverlay, currentMode);
        // Не очищаем код, просто меняем парсер на бэкенде
    });
    
    // Загрузка справки
    fetch('/api/help')
        .then(r => r.json())
        .then(data => {
            const list = data[currentMode];
            els.helpList.innerHTML = list.map(item => `
                <div class="help-item">
                    ${item.ru ? `<span class="help-ru">${item.ru}</span> → ` : ''}
                    <span class="help-en">${item.en}</span>
                    <div class="help-desc">${item.desc}</div>
                </div>
            `).join('');
        });
    
    // Рендер регистров (статично, обновляем значения динамически)
    function renderRegisters() {
        let html = '';
        for (let i = 0; i < 32; i++) {
            const name = i === 0 ? 'zero' : `x${i}`;
            html += `<div class="reg-item" id="reg-${i}">
                <span class="reg-name">${name}</span>: 
                <span class="reg-value" id="reg-val-${i}">0</span>
            </div>`;
        }
        els.registersGrid.innerHTML = html;
    }
    renderRegisters();
    
    // Рендер памяти (первые 256 байт)
    function renderMemory(memArray) {
        if (!memArray) return;
        els.memoryView.innerHTML = memArray.slice(0, 256).map((byte, i) => 
            `<div class="mem-cell" id="mem-${i}">${byte.toString(16).padStart(2,'0')}</div>`
        ).join('');
    }
    
    // === API вызовы ===
    
    async function sendToSimulator(action) {
        const payload = {
            code: els.editor.value,
            mode: currentMode,
            action: action
        };
        
        try {
            const response = await fetch('/api/execute', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(payload)
            });
            
            const result = await response.json();
            
            if (result.success === false) {
                els.outputLog.textContent += `\n[ERROR] ${result.error}`;
                return null;
            }
            
            // Обновляем UI
            updateUI(result);
            return result;
            
        } catch (err) {
            els.outputLog.textContent += `\n[NETWORK] ${err.message}`;
            return null;
        }
    }
    
    function updateUI(result) {
        // PC
        els.pcValue.textContent = `0x${result.pc.toString(16).padStart(8, '0')}`;
        
        // Регистры (с анимацией изменений)
        for (let i = 0; i < 32; i++) {
            const el = document.getElementById(`reg-val-${i}`);
            const newVal = result.registers[i] || 0;
            const oldVal = parseInt(el.textContent) || 0;
            
            if (newVal !== oldVal) {
                el.textContent = newVal;
                el.classList.add('changed');
                setTimeout(() => el.classList.remove('changed'), 300);
            }
        }
        
        // Память
        if (result.memory) {
            renderMemory(result.memory);
        }
        
        // Подсветка текущей строки в редакторе
        highlightCurrentLine(result.line);
        
        // Лог
        if (result.output) {
            els.outputLog.textContent += `\n${result.output}`;
            els.outputLog.scrollTop = els.outputLog.scrollHeight;
        }
    }
    
    function highlightCurrentLine(lineIndex) {
        // Удаляем старые подсветки
        document.querySelectorAll('.highlight-line').forEach(el => 
            el.classList.remove('highlight-line'));
        
        if (lineIndex >= 0) {
            // Простая реализация: считаем строки по \n
            const lines = els.editor.value.split('\n');
            if (lines[lineIndex]) {
                // В реальном проекте лучше использовать CodeMirror-подобный подход
                // Для демо: просто скроллим и добавляем класс к строке
                els.outputLog.textContent += `\n[STEP] Выполняется строка ${lineIndex + 1}`;
            }
        }
    }
    
    // === Обработчики кнопок ===
    
    els.btnStep.addEventListener('click', () => {
        els.outputLog.textContent = ''; // Очищаем лог для шага
        sendToSimulator('step');
    });
    
    els.btnRun.addEventListener('click', () => {
        els.outputLog.textContent = '=== Запуск программы ===\n';
        sendToSimulator('run');
    });
    
    els.btnReset.addEventListener('click', () => {
        sendToSimulator('reset');
        els.outputLog.textContent = '[RESET] Сброшено';
        // Сброс визуализации
        els.pcValue.textContent = '0x00000000';
        document.querySelectorAll('.reg-value').forEach(el => {
            el.textContent = '0';
            el.classList.remove('changed');
        });
    });
    
    // === Визуальный режим (Blockly) ===
    
    els.btnVisual.addEventListener('click', () => {
        initBlockly();
        els.visualOverlay.classList.remove('hidden');
    });
    
    els.btnCloseVisual.addEventListener('click', () => {
        els.visualOverlay.classList.add('hidden');
    });
    
    els.btnApplyBlocks.addEventListener('click', () => {
        if (!blocklyWorkspace) return;
        
        const generator = currentMode === 'russian' 
            ? Blockly.RiscvRu 
            : Blockly.RiscvEn;
            
        const code = generator.workspaceToCode(blocklyWorkspace);
        els.editor.value = code;
        
        // Обновляем подсветку
        SyntaxHighlighter.bind(els.editor, els.syntaxOverlay, currentMode);
        
        els.visualOverlay.classList.add('hidden');
    });
    
    function initBlockly() {
        if (blocklyWorkspace) return; // Уже инициализировано
        
        blocklyWorkspace = Blockly.inject('blockly-area', {
            toolbox: getToolboxDefinition(),
            scrollbars: true,
            trashcan: true,
            grid: { spacing: 20, length: 3, colour: '#333', snap: true }
        });
        
        // Загружаем определения блоков
        Blockly.defineBlocksWithJsonArray(getBlockDefinitions());
    }
    
    // === Вкладки помощи/вывода ===
    els.tabs.forEach(btn => {
        btn.addEventListener('click', () => {
            const tab = btn.dataset.tab;
            els.tabs.forEach(b => b.classList.remove('active'));
            els.tabContents.forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            document.getElementById(`${tab}-tab`).classList.add('active');
        });
    });
});
