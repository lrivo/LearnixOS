cc = meson.get_compiler('c')

userspace_c_args = [
  '-ffreestanding',
  '-nostdlib',
  '-nostdinc',
  '-fstack-protector',  # enables stack canaries
  '-fno-PIC',
  '-mno-red-zone',
]

userspace_link_args = [
  '-nostdlib',
  '-nostdinc',
  '-static',
  '-z', 'max-page-size=0x1000',
  '-T', meson.current_source_dir() / 'userspace.ld'
]

# userspace programs to build
subdir('init')
subdir('sh')
subdir('echo')
subdir('test')
subdir('brainfuck')
