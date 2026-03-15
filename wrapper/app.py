#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flask-обёртка для C++ симулятора.
Минимальное потребление памяти: без ORM, без кэшей, только subprocess.
"""

import os
import sys
import json
import subprocess
from flask import Flask, request, jsonify, render_template

app = Flask(__name__, static_folder='../static', template_folder='../templates')

# Путь к скомпилированному ядру
SIM_BINARY = os.path.join(os.path.dirname(__file__), '../core/riscv_sim')

@app.route('/')
def index():
    """Главная страница"""
    return render_template('index.html')

@app.route('/api/execute', methods=['POST'])
def execute():
    """
    Единая точка API.
    Вход: {"code": "строка кода", "mode": "russian|english", "action": "step|run|reset"}
    Выход: {"registers": {...}, "pc": int, "memory": {...}, "output": "...", "error": "...", "line": int}
    """
    try:
        data = request.get_json(force=True)
        code = data.get('code', '')
        mode = data.get('mode', 'english')
        action = data.get('action', 'step')
        
        # Разбиваем код на строки
        lines = [l.strip() for l in code.split('\n') if l.strip()]
        
        # Запускаем симулятор как subprocess
        # Передаём код через stdin, читаем JSON из stdout
        proc = subprocess.Popen(
            [SIM_BINARY, '--json-output'],  # Флаг для вывода в JSON
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        
        # Формируем входные данные для C++
        input_data = {
            'lines': lines,
            'mode': mode,
            'action': action
        }
        
        stdout, stderr = proc.communicate(
            input=json.dumps(input_data),
            timeout=30  # Защита от зависаний
        )
        
        if proc.returncode != 0:
            return jsonify({
                'success': False,
                'error': f'Симулятор упал: {stderr}',
                'registers': {}, 'pc': 0, 'memory': {}, 'output': '', 'line': -1
            }), 500
        
        result = json.loads(stdout)
        return jsonify(result)
        
    except subprocess.TimeoutExpired:
        return jsonify({'success': False, 'error': 'Таймаут выполнения'}), 504
    except json.JSONDecodeError as e:
        return jsonify({'success': False, 'error': f'Ошибка парсинга ответа: {e}'}), 500
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/help')
def get_help():
    """Справочник команд для правой панели"""
    return jsonify({
        'russian': [
            {'ru': 'сложи', 'en': 'add', 'desc': 'Сложение: rd = rs1 + rs2'},
            {'ru': 'вычти', 'en': 'sub', 'desc': 'Вычитание: rd = rs1 - rs2'},
            {'ru': 'загрузи', 'en': 'lw', 'desc': 'Загрузка слова из памяти'},
            {'ru': 'сохрани', 'en': 'sw', 'desc': 'Сохранение слова в память'},
            {'ru': 'ветви_если_равно', 'en': 'beq', 'desc': 'Переход если регистры равны'},
        ],
        'english': [
            {'en': 'add', 'desc': 'Addition: rd = rs1 + rs2'},
            {'en': 'sub', 'desc': 'Subtraction: rd = rs1 - rs2'},
            {'en': 'lw', 'desc': 'Load word from memory'},
            {'en': 'sw', 'desc': 'Store word to memory'},
            {'en': 'beq', 'desc': 'Branch if equal'},
        ]
    })

if __name__ == '__main__':
    # Запуск на всех интерфейсах, порт 5000
    # Для продакшена использовать gunicorn: gunicorn -w 1 -b 0.0.0.0:5000 app:app
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)
