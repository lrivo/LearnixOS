project('learnixos', 'c', 'nasm')

# 0. make sure dependencies are installed
cc_runtime_dep = files('deps/cc-runtime/src/cc-runtime.c')
freestnd_hdrs_dep  = include_directories('deps/freestnd-c-hdrs/include/', is_system:true)
limine_hdrs_dep = include_directories('deps/limine-protocol/include/')

# 1. read user selected componets
kmalloc = get_option('kmalloc')
sched = get_option('sched')
rand = get_option('rand')
pmm = get_option('pmm')

# 2. compile and link the project
subdir('kernel')
subdir('liblearnix')
subdir('userspace')

# 3. construct the bootable iso
xorriso = find_program('xorriso')
limine = find_program('deps/limine/limine')

iso_deps = [
  kernel_elf,
  init_elf,
  sh_elf,
  echo_elf,
  test_elf,
  brainfuck_elf,
]

iso_target = custom_target(
  'iso_target',
  input: iso_deps,
  output: 'learnixos.iso',
  depends: iso_deps,
  command: [
    find_program('sh'),
    meson.project_source_root() / 'make_iso.sh',
    xorriso.full_path(),
    limine.full_path(),
    '@PRIVATE_DIR@',
    meson.project_source_root(),
    '@OUTPUT@',
    '@INPUT@',
  ],
  build_by_default: true
)

# 4. launch QEMU
qemu = find_program('qemu-system-x86_64')

run_target('run',
  command: [
    qemu,
    '-M', 'q35',
    '-drive', 'if=pflash,unit=0,format=raw,file=' + meson.project_source_root() / 'deps/edk2-ovmf/ovmf-code-x86_64.fd,readonly=on',
    '-cdrom', iso_target,
    '-boot', 'd',
    '-no-reboot', 
    '-no-shutdown',
    '-m', '2G',
    '-d', 'cpu_reset',
    '-cpu', 'qemu64,+rdrand,+rdseed',
  ],
  depends: iso_target
)


run_target('gdb',
  command: [
    qemu,
    '-M', 'q35',
    '-drive', 'if=pflash,unit=0,format=raw,file=' + meson.project_source_root() / 'deps/edk2-ovmf/ovmf-code-x86_64.fd,readonly=on',
    '-cdrom', iso_target,
    '-boot', 'd',
    '-no-reboot', 
    '-no-shutdown',
    '-m', '2G',
    '-d', 'cpu_reset',
    '-cpu', 'qemu64,+rdrand,+rdseed',
    '-s', '-S',
  ],
  depends: iso_target
)
