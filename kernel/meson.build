# === Architecture Independent === #
kernel_c_args = [
  '-Wall',
  '-Wextra',
  '-std=gnu11',
  '-O2',
  '-ffreestanding',
  '-nostdlib',
  '-nostdinc',
  '-fno-stack-protector',     # disable stack canaries
  '-fno-stack-check',
  '-fno-omit-frame-pointer',  # always use the RBP register
  '-fno-PIC',
  '-fno-lto',
  '-ffunction-sections',
  '-fdata-sections',
  '-mno-80387',
  '-mno-mmx',
  '-mno-sse',
  '-mno-sse2',
  '-mno-red-zone',    # learnix doesn't support the red zone optimization
  '-mcmodel=kernel',
  '-D', 'PRINTF_DISABLE_SUPPORT_FLOAT',
  '-D', 'PRINTF_DISABLE_SUPPORT_EXPONENTIAL',
  '-D', 'PRINTF_DISABLE_SUPPORT_LONG_LONG',
  '-D', 'PRINTF_DISABLE_SUPPORT_PTRDIFF_T',
  '-g',  # TODO:disable in production
  '-pipe',
]

kernel_nasm_args = [
  '-F', 'dwarf',
  '-g',
]

kernel_link_args = [
  '-nostdlib',
  '-static',
  '-z', 'max-page-size=0x1000',
  #'--gc-sections',
]

kernel_src = files(
  'main.c',
  'kpanic.c',
  'acpi/acpi.c',
  'drivers/console/console.c',
  'drivers/input/ps2kb.c',
  'drivers/serial.c',
  'lib/debug.c',
  'lib/elf.c',
  'lib/kprintf.c',
  'lib/limine_module.c',
  'mm/uvm.c',
  'mm/vmm.c',
  'proc/execve.c',
  'proc/fork.c',
  'proc/getpid.c',
  'proc/process.c',
  'proc/wait.c',
  'sched/sched.c',
  'sys/syscall.c',
  'sys/tty.c',
  'security/canary.c',
)
kernel_src += cc_runtime_dep

kernel_include_dirs = [
  include_directories('include'),
  freestnd_hdrs_dep,
  limine_hdrs_dep,
]

# === User defined components === #
if sched == 'round_robin'
  kernel_src += 'sched/rr/rr.c'
elif sched == 'lottery'
  kernel_src += 'sched/lottery/lottery.c'
endif

if pmm == 'bitmap'
  kernel_src += 'mm/pmm_bitmap.c'
endif

if kmalloc == 'watermark'
  kernel_src += 'mm/kmalloc_watermark.c'
elif kmalloc == 'free_list' 
  kernel_src += 'mm/kmalloc_freelist.c'
endif

if rand == 'chacha20'
  kernel_src += 'security/chacha20.c'
endif

# === Architecture Dependent === #
arch = host_machine.cpu_family()
if arch == 'x86_64'
    subdir('arch/amd64')
    kernel_src += amd64_src
    kernel_c_args += amd64_c_args
    kernel_nasm_args += amd64_nasm_args
    kernel_link_args += amd64_link_args
    kernel_include_dirs += amd64_include_dirs
endif

# === PSF2 font binary object == #
objcopy = find_program('objcopy')
font_obj = custom_target('font_object',
  input: 'default8x16.psfu',
  output: 'font.o',
  command: [
    objcopy,
    '-O', 'elf64-x86-64',
    '-B', 'i386',
    '-I', 'binary',
    '--redefine-sym', '_binary____kernel_default8x16_psfu_start=_binary_default8x16_psfu_start',
    '--redefine-sym', '_binary____kernel_default8x16_psfu_end=_binary_default8x16_psfu_end',
    '--redefine-sym', '_binary____kernel_default8x16_psfu_size=_binary_default8x16_psfu_size',
    '@INPUT@',
    '@OUTPUT@'
  ]
)

# === Executable output === #
kernel_elf = executable(
  'kernel.elf',
  kernel_src,
  font_obj,
  c_args: kernel_c_args,
  nasm_args: kernel_nasm_args,
  link_args: kernel_link_args,
  include_directories: kernel_include_dirs
)
