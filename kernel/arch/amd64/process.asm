BITS 64

global test_ucode
test_ucode:
.loop: 
  inc rax
  jmp .loop

; rdi = struct trapframe *tf
global arch_test_jump_usermode
arch_test_jump_usermode:
  mov rsp, rdi

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
  
  sti
  iretq
