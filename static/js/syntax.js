/**
 * Минималистичная подсветка синтаксиса для ассемблера.
 * Без внешних зависимостей, работает на регулярках.
 */
const SyntaxHighlighter = {
    keywords: {
        russian: ['сложи','вычти','и','или','исключи_или','сдвинь_влево','сдвинь_вправо',
                  'загрузи','сохрани','ветви_если_равно','ветви_если_не_равно',
                  'перейти','перейти_рег','системный','nop'],
        english: ['add','sub','and','or','xor','sll','srl','lw','sw','beq','bne',
                  'jal','jalr','ecall','nop']
    },
    
    registers: ['x\\d+','zero','ra','sp','gp','tp','t[0-6]','s[0-9]','a[0-7]','fp'],
    
    highlight(text, mode = 'russian') {
        const keywords = this.keywords[mode] || this.keywords.english;
        let html = text
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;');
        
        // Комментарии
        html = html.replace(/(\/\/[^\n]*)/g, '<span class="comment">$1</span>');
        
        // Ключевые слова
        const kwPattern = new RegExp(`\\b(${keywords.join('|')})\\b`, 'g');
        html = html.replace(kwPattern, '<span class="keyword">$1</span>');
        
        // Регистры
        const regPattern = new RegExp(`\\b(${this.registers.join('|')})\\b`, 'g');
        html = html.replace(regPattern, '<span class="register">$1</span>');
        
        // Числа (шестнадцатеричные и десятичные)
        html = html.replace(/\b(0x[0-9a-fA-F]+|\d+)\b/g, '<span class="number">$1</span>');
        
        return html;
    },
    
    // Привязка к textarea с синхронизацией скролла
    bind(textarea, overlay, mode) {
        const update = () => {
            overlay.innerHTML = this.highlight(textarea.value, mode);
            overlay.scrollTop = textarea.scrollTop;
            overlay.scrollLeft = textarea.scrollLeft;
        };
        
        textarea.addEventListener('input', update);
        textarea.addEventListener('scroll', () => {
            overlay.scrollTop = textarea.scrollTop;
            overlay.scrollLeft = textarea.scrollLeft;
        });
        
        // Инициализация
        update();
    }
};
