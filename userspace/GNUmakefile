# LLVM/clang as the target compiler
CC := clang 
LD := ld.lld 

# C compilation flags
CFLAGS := -ffreestanding \
	-static \
	-nostdlib \
	-target x86_64-unknown-none-elf \
	-fno-stack-protector \
	-mno-red-zone \
	-fno-PIC \
	-I ../liblearnix/include

# linker flags
LDFLAGS := -m elf_x86_64 \
	-nostdlib \
	-static \
	-z max-page-size=0x1000 \
	-T userspace.ld	

all:
	$(CC) $(CFLAGS) -c hello.c -o hello.c.o
	$(LD) $(LDFLAGS) hello.c.o ../liblearnix/liblearnix.a -o hello.o

clean:
	rm *.o
