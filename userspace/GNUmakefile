OUT := out
CC := clang 
LD := ld.lld 

PROGRAMS := $(filter-out out,$(patsubst %/,%,$(wildcard */)))

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

LDSCRIPT := userspace.ld
LIBC := ../liblearnix/liblearnix.a

.PHONY: all clean $(PROGRAMS)

all: $(OUT) $(PROGRAMS)

$(OUT):
	mkdir -p $(OUT)

$(PROGRAMS):
	$(CC) $(CFLAGS) -c $@/$@.c -o $@/$@.o
	$(LD) -T $(LDSCRIPT) $@/$@.o $(LIBC) -o $(OUT)/$@

clean:
	rm -rf $(OUT)
	find . -name "*.o" -delete
