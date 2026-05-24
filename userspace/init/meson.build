init_elf = executable(
  'init.elf', 
  ['init.c', userspace_files],
  c_args: userspace_c_args,
  link_args: userspace_link_args,
  dependencies: liblearnix_dep,
)
