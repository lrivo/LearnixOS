# LLVM/clang as the target compiler
CC := clang
ASM := nasm

# C compilation flags
CFLAGS := -ffreestanding \
	-static \
	-nostdlib \
	-target x86_64-unknown-none-elf \
	-fno-omit-frame-pointer \
	-fno-stack-protector \
	-mno-red-zone \
	-fno-PIC

NASMFLAGS := -f elf64 -Wall

all:
	# compile .c files
	$(CC) $(CFLAGS) -c liblearnix.c -o liblearnix.c.o
	# assemble .asm files
	nasm $(NASMFLAGS)	liblearnix.asm -o liblearnix.asm.o
	# create the archive
	llvm-ar rcs liblearnix.a *.o

clean:
	rm -f *.o
	rm -f *.a
