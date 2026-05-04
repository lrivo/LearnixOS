#pragma once
#include <learnix/types.h>

/* ELF64 types. */
typedef uint16_t elf64_half;
typedef int16_t  elf64_shalf;
typedef uint32_t elf64_word;
typedef int32_t  elf64_sword;
typedef uint64_t elf64_xword;
typedef int64_t  elf64_sxword;
typedef uint64_t elf64_addr;
typedef uint64_t elf64_off;

/* ELF constants */
#define ELF_MAGIC "\177ELF"

/* https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-43405.html */
struct elf64_hdr
{
  /* contains the ELF_MAGIC identifier. */
  #define EI_MAG0     0
  #define EI_MAG1     1
  #define EI_MAG2     2
  #define EI_MAG3     3
  #define EI_CLASS    4
  #define EI_DATA     5
  #define EI_VERSION  6
  #define EI_OSABI    7
  #define EI_PAD      8
  unsigned char e_ident[16];
  
  /* identifies the object file type. */
  #define ET_NONE 0 // no file type
  #define ET_REL  1 // relocatable file
  #define ET_EXEC 2 // executable file
  #define ET_DYN  3 // shared object file
  elf64_half e_type;
  
  /* specifies the target architecture. */
  #define EM_NONE  0  // no machine
  #define EM_AMD64 62 // x86_64
  elf64_half e_machine;

  /* identifies the object file version. */
  #define EV_NONE 0
  #define EV_CURRENT 1  // at least >1
  elf64_word e_version;
  
  /* the first userspace virtual address to execute. */
  elf64_addr e_entry;
  
  /* the program header table's offset in bytes. */
  elf64_off e_phoff;
  
  /* the section header table's offset in bytes. */
  elf64_off e_shoff;
  
  /* processor-specific flags associated with the file. */
  elf64_word e_flags;
  
  /* the ELF header's size in bytes. */
  elf64_half e_ehsize;
  
  /* the size in bytes of an entry in the program header table. */
  elf64_half e_phentsize;
  
  /* the number of entries in the program header table. */
  elf64_half e_phnum;

  /* the size in bytes of an entry in the section table. */
  elf64_half e_shentsize;

  /* the number of entries in the section table. */
  elf64_half e_shnum;
  
  /* The section header table index of the entry that 
     is associated with the section name string table. */
  elf64_half e_shstrndx;
};

/* ELF64 Program Header Table */
struct elf64_phdr
{
  #define PT_NULL 0 // unused value
  #define PT_LOAD 1 // a loadable segment (execve only cares about this)
  elf64_word p_type;
  
  /* Specifies memory mapping permissions (will overlap). */
  #define PF_X 1   // executable
  #define PF_W 2   // writable
  #define PF_R 4   // readable
  elf64_word p_flags;
  
  /* The offset from the beginning of the file at which the first byte of the segment resides. */
  elf64_off p_offset;
  
  /* The virtual address at which the first byte of the segment resides in memory. */
  elf64_addr p_vaddr;
  
  /* The segment's physical address for systems in which physical addressing is relevant. */
  elf64_addr p_paddr; // NOTE: we don't care
  
  /* The number of bytes in the file image of the segment. */
  elf64_xword p_filesz;
  
  /* The number of bytes in the memory image of the segment. */
  elf64_xword p_memsz;
  
  /* Specifies page alignment (power of two). */
  elf64_xword p_align;
};
