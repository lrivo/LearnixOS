BITS 64
section .text

; kernel entrypoint to handle syscalls
global syscall_handler
syscall_handler:
  swapgs
  mov [gs:0x00], rsp  ; gs.user_gs = rsp      (save userland rsp) 
  mov rsp, [gs:0x08]  ; rsp = gs.kern_stack   (switch to the kstack)
  
  push r11            ; saved userland rflags
  push rcx            ; saved userland rip
  
  mov rsp, [gs:0x00]  ; restore userland rsp 
  swapgs
  sti                 ; re-enable interrupts
  sysretq             ; return to userspace

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
.loop: jmp .loop
