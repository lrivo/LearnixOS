# LLVM/clang as the target compiler
CC := clang 
LD := ld.lld 

# C compilation flags
CFLAGS := -ffreestanding \
	-static \
	-nostdlib \
	-target x86_64-unknown-none-elf \
	-fno-stack-protector \
	-fno-PIC

# linker flags
LDFLAGS := -m elf_x86_64 \
	-nostdlib \
	-static \
	-z max-page-size=0x1000

all:
	$(CC) $(CFLAGS) hello.c -o hello.o

clean:
	rm *.o
