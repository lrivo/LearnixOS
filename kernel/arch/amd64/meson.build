# Source files for this architecture
amd64_src = files(
  'cpu.c',
  'exit.asm',
  'gdt.c',
  'idt.c',
  'ioapic.c',
  'isr_stubs.asm',
  'lapic.c',
  'paging.c',
  'process.asm',
  'process.c',
  'string.c',
  'syscall.asm',
  'rdseed.asm',
)

# Architecture specific compilation/linker arguments
amd64_c_args = [
  '-mabi=sysv',
  '-m64'
]

amd64_nasm_args = [
  '-f', 'elf64',
]

amd64_link_args = [
  '-T', meson.current_source_dir() / 'x86_64.lds',
]

amd64_include_dirs = [
  include_directories('include')
]
