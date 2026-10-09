#!/bin/bash

echo "=== LOTOS OS BUILD ==="


# ===== KOMPILACJA =====

echo "[1/9] Kompilacja boot.asm..."
nasm -f elf32 src/boot.asm -o boot.o || exit 1

echo "[2/9] Kompilacja logo.bmp..."
objcopy -I binary -O elf32-i386 -B i386 logo.bmp logo.o || exit 1

echo "[3/9] Kompilacja kernel.c..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector \
    -c src/kernel.c -o kernel.o || exit 1

echo "[4/9] Kompilacja commands.c..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector \
    -c src/commands.c -o commands.o || exit 1

echo "[5/9] Kompilacja games.c..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector \
    -c src/games.c -o games.o || exit 1

echo "[6/9] Kompilacja usb.c..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector \
    -c src/usb.c -o usb.o || exit 1

echo "[7/9] Kompilacja disk.c..."
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector \
    -c src/disk.c -o disk.o || exit 1

echo "[8/9] Linkowanie..."
ld -m elf_i386 -T linker.ld -o kernel.bin \
    boot.o \
    logo.o \
    kernel.o \
    commands.o \
    games.o \
    usb.o \
    disk.o || exit 1

echo "[9/9] Tworzenie ISO..."
cp kernel.bin iso/boot/kernel.bin || exit 1

grub-mkrescue -o lotos.iso iso || exit 1

echo ""
echo "=============================="
echo "       LOTOS OS GOTOWY!"
echo "=============================="
echo ""

qemu-system-i386 -cdrom lotos.iso
