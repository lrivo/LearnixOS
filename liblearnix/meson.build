liblearnix_c_args = [
  '-ffreestanding',
  '-nostdlib',
  '-fno-omit-frame-pointer',
  '-fno-stack-protector',
  '-fno-PIC',
  '-mno-red-zone'
]

liblearnix_nasm_args = [
  '-f', 'elf64',
  '-F', 'dwarf',
]

liblearnix_src = files(
  'liblearnix.c',
  'liblearnix.asm',
)

# output: liblearnix.a
liblearnix_ar = static_library(
  'liblearnix',
  liblearnix_src,
  c_args: liblearnix_c_args,
  nasm_args: liblearnix_nasm_args
)

liblearnix_dep = declare_dependency(
  link_with: liblearnix_ar,
  include_directories: include_directories('include')
)
