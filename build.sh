#!/bin/bash
set -e

echo "🔧 Сборка RISC-V симулятора для слабого железа..."

# 1. Собираем C++ ядро
echo "→ Компиляция C++ ядра..."
cd core
make clean && make
cd ..

# 2. Минифицируем Blockly (если не скачан предварительно)
if [ ! -f static/blockly/blockly_compressed.js ]; then
    echo "→ Загрузка Blockly (минифицированная версия)..."
    mkdir -p static/blockly
    # Скачиваем с CDN минимальный набор (~200 КБ вместо 2 МБ)
    curl -s "https://unpkg.com/blockly/blockly_compressed.js" -o static/blockly/blockly_compressed.js
    curl -s "https://unpkg.com/blockly/blocks_compressed.js" -o static/blockly/blocks_compressed.js
    curl -s "https://unpkg.com/blockly/javascript_compressed.js" -o static/blockly/javascript_compressed.js
fi

# 3. Проверяем зависимости Python
echo "→ Проверка Python зависимостей..."
pip3 install -q flask 2>/dev/null || pip install -q flask

echo "✅ Сборка завершена!"
echo ""
echo "📌 Запуск:"
echo "   cd wrapper && python3 app.py"
echo "   Затем откройте в браузере: http://localhost:5000"
echo ""
echo "⚡ Для продакшена:"
echo "   pip install gunicorn"
echo "   gunicorn -w 1 -b 0.0.0.0:5000 --timeout 60 wrapper.app:app"
