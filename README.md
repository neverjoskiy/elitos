# elitos

Учебное гибридное ядро ОС для x86, написанное на C и Assembly (95/5).

## Что есть

- Загрузчик (512 байт, real mode → protected mode)
- GDT, IDT, paging
- PS/2 keyboard driver + PIC 8259A
- Терминал с вводом (readline, backspace, аппаратный курсор)
- Round-robin планировщик
- Виртуальная файловая система (in-memory)
- Простая СУБД (таблицы, индексы, транзакции)
- Модульная архитектура
- Shell с командами: `help`, `ls`, `cat`, `echo`

## Сборка и запуск

### Зависимости

**Arch Linux:**
```bash
sudo pacman -S gcc nasm binutils make qemu-system-x86
```

**Ubuntu/Debian:**
```bash
sudo apt install gcc nasm binutils make qemu-system-x86
```

### Запуск

```bash
make clean && make
make run
```

Или через образ диска:
```bash
make disk
qemu-system-i386 -fda disk.img
```

## Структура

```
boot/        — загрузчик и переход в protected mode
kernel/      — исходники ядра (C)
include/     — заголовочные файлы
Makefile     — сборка
```

## TODO

- [ ] Клавиатурный ввод — готово
- [ ] kmalloc
- [ ] Системные вызовы
- [ ] FAT12/16 (персистентная ФС)
- [ ] User mode
- [ ] Сеть
