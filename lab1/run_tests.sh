#!/bin/bash

# Тестовые данные
cat > test_input.txt << EOF
build/file1.txt
build/file2.txt
hello
very_long_string_here
world
another_long_string
test
EOF

# Ожидаемые результаты
cat > expected1.txt << EOF
olleh
dlrow
tset
EOF

cat > expected2.txt << EOF
ereh_gnirts_gnol_yrev
gnirts_gnol_renhtona
EOF

# Запуск
./build/client < test_input.txt

# Проверка
#echo "=== Результаты ==="
#echo "build/file1.txt:"
#cat build/file1.txt
#echo ""
#echo "build/file2.txt:"
#cat build/file2.txt
#echo ""
#
#echo "=== Сравнение ==="
#if diff build/file1.txt expected1.txt > /dev/null; then
#    echo "✓ build/file1.txt совпадает"
#else
#    echo "✗ build/file1.txt не совпадает"
#    diff build/file1.txt expected1.txt
#fi
#
#if diff build/file2.txt expected2.txt > /dev/null; then
#    echo "✓ build/file2.txt совпадает"
#else
#    echo "✗ build/file2.txt не совпадает"
#    diff build/file2.txt expected2.txt
#fi

# Очистка
rm -f test_input.txt expected1.txt expected2.txt
