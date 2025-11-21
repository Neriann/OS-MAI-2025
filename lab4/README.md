# Сборка программ с динамическими библиотеками
---

## Создание динамических библиотек

```bash
gcc -o libcustom1.so -shared -fPIC library1.c
gcc -o libcustom1.so -shared -fPIC library2.c
```

## Сборка программы со статической линковкой

```bash
# Линкуем нужную библиотеку
gcc -o test_static libcustom1.so test_static_library.c
# или же
gcc -o test_static -L. -lcustom1 test_static_library.c
```

## Запуск программы

```bash
LD_LIBRARY_PATH=. ./test_static
```

LD_LIBRARY_PATH - переменная окружения, подсказывающая динамическому загрузчику где искать динамические библиотеки,
находящиеся не в системных путях (/lib или /usr/lib)

## Сборка программы с динамической линковкой

```bash
# Подключаем динамический линковщик
gcc -o test_dynamic -ldl test_dynamic_library.c
```

## Запуск программы

```bash
LD_LIBRARY_PATH=. ./test_dynamic ./libcustom1.so ./libcustom2.so
```
