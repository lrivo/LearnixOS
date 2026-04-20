BITS 64

global test_ucode
test_ucode:
  mov rax, 1
  mov r15, 15
.loop: 
  jmp .loop

; rdi = struct arch_proc_context *prev 
; rsi = struct arch_proc_context *next
global _switch_to
_switch_to:
  ; save registers on prev kernel stack
  push rbx
  push rbp
  push r12
  push r13
  push r14
  push r15
  
  ; save current stack pointer 
  ; prev->ctx.rsp = prev's rsp 
  mov [rdi], rsp
  
  ; load next's stack pointer 
  ; rsp = next->ctx.rsp 
  mov rsp, [rsi]
  
  ; now we are on next's kernel stack
  pop r15 
  pop r14
  pop r13
  pop r12
  pop rbp
  pop rbx
  
  ; back to arch_context_switch
  ret
  
; rdi = struct trapframe *tf
global jump_usr_first_time
jump_usr_first_time:
  ; pop the initial GPR values that arch_proc_init()
  ; has placed on the kernel stack
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdi
  pop rsi
  pop rbp
  pop rdx 
  pop rcx 
  pop rbx
  pop rax
 
  add rsp, 16   ; skip error and vector_num
  iretq         ; jump in userspace with interrupts enabled
