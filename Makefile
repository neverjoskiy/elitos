CC = gcc
AS = nasm
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -I./include
ASFLAGS = -f elf32

all: kernel.bin

boot/bootloader.bin: boot/bootloader.asm
	$(AS) -f bin -o boot/bootloader.bin boot/bootloader.asm

kernel/kernel.elf: boot/protected_mode.o kernel/main.o kernel/kernel.o kernel/terminal.o kernel/keyboard.o kernel/scheduler.o kernel/vfs.o kernel/commands.o kernel/db.o kernel/modules.o kernel/gdt.o kernel/idt.o kernel/paging.o
	$(CC) $(CFLAGS) -T kernel/linker.ld -o kernel/kernel.elf $^

kernel.bin: kernel/kernel.elf
	objcopy -O binary kernel/kernel.elf kernel.bin

boot/protected_mode.o: boot/protected_mode.asm
	$(AS) $(ASFLAGS) -o boot/protected_mode.o boot/protected_mode.asm

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c -o kernel/kernel.o kernel/kernel.c

kernel/terminal.o: kernel/terminal.c
	$(CC) $(CFLAGS) -c -o kernel/terminal.o kernel/terminal.c

kernel/scheduler.o: kernel/scheduler.c
	$(CC) $(CFLAGS) -c -o kernel/scheduler.o kernel/scheduler.c

kernel/vfs.o: kernel/vfs.c
	$(CC) $(CFLAGS) -c -o kernel/vfs.o kernel/vfs.c

kernel/commands.o: kernel/commands.c
	$(CC) $(CFLAGS) -c -o kernel/commands.o kernel/commands.c

kernel/db.o: kernel/db.c
	$(CC) $(CFLAGS) -c -o kernel/db.o kernel/db.c

kernel/modules.o: kernel/modules.c
	$(CC) $(CFLAGS) -c -o kernel/modules.o kernel/modules.c

kernel/gdt.o: kernel/gdt.c
	$(CC) $(CFLAGS) -c -o kernel/gdt.o kernel/gdt.c

kernel/idt.o: kernel/idt.c
	$(CC) $(CFLAGS) -c -o kernel/idt.o kernel/idt.c

kernel/paging.o: kernel/paging.c
	$(CC) $(CFLAGS) -c -o kernel/paging.o kernel/paging.c

kernel/keyboard.o: kernel/keyboard.c
	$(CC) $(CFLAGS) -c -o kernel/keyboard.o kernel/keyboard.c

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c -o kernel/main.o kernel/main.c

clean:
	rm -f boot/*.o boot/*.elf boot/*.bin kernel/*.o kernel/*.elf kernel.bin

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin

disk: kernel.bin
	dd if=/dev/zero of=disk.img bs=512 count=2880
	dd if=boot/bootloader.bin of=disk.img conv=notrunc
	dd if=kernel.bin of=disk.img seek=1 conv=notrunc

iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	echo 'set timeout=0'                          > iso/boot/grub/grub.cfg
	echo 'set default=0'                         >> iso/boot/grub/grub.cfg
	echo 'menuentry "elitos" {'                  >> iso/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.bin'        >> iso/boot/grub/grub.cfg
	echo '    boot'                              >> iso/boot/grub/grub.cfg
	echo '}'                                     >> iso/boot/grub/grub.cfg
	grub-mkrescue -o elitos.iso iso

run-iso: elitos.iso
	qemu-system-i386 -cdrom elitos.iso

.PHONY: all clean run disk iso run-iso
