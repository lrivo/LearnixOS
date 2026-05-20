project('learnix-userspace', 'c', version: '0.1')

cc = meson.get_compiler('c')

liblearnix_inc = include_directories('../liblearnix/include')
liblearnix_dep = cc.find_library('liblearnix', dirs: [meson.current_source_dir() / '../liblearnix'])

userspace_c_args = [
  '-ffreestanding',
  '-nostdlib',
  '-fno-stack-protector',
  '-fno-PIC',
  '-mno-red-zone',
]

userspace_link_args = [
  '-nostdlib',
  '-static',
  '-z', 'max-page-size=0x1000',
  '-T', meson.current_source_dir() / 'userspace.ld'
]

subdir('init')
subdir('sh')
subdir('echo')
subdir('test')
subdir('brainfuck')
