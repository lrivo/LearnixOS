project('learnixos', 'c', 'nasm')

# 1. compile and link the project
subdir('kernel')
subdir('liblearnix')
subdir('userspace')

# 2. construct the bootable iso
xorriso = find_program('xorriso')
limine = find_program('limine/limine')

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

# 3. run targets
qemu = find_program('qemu-system-x86_64')

run_target('run-uefi',
  command: [
    qemu,
    '-M', 'q35',
    '-drive', 'if=pflash,unit=0,format=raw,file=' + meson.project_source_root() / 'edk2-ovmf/ovmf-code-x86_64.fd,readonly=on',
    '-cdrom', iso_target,
    '-boot', 'd',
    '-no-reboot', 
    '-no-shutdown',
    '-m', '2G',
    '-d', 'cpu_reset'
  ],
  depends: iso_target
)
