BITS 64
section .text

; jump_usermode(void *rip, void *rsp)
; rdi = rip, rsi = rsp
global jump_usermode
jump_usermode:
  mov ax, 0x18 | 3
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push 0x18 | 3
  push rsi
  pushfq
  push 0x20 | 3
  push rdi
  iretq

global usermode_test
usermode_test:
  mov rax, 1
  syscall
