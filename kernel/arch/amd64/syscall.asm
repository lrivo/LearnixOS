BITS 64
section .text

; C function that selects the correct syscall handler
extern syscall_dispatcher

; x86_64 syscall calling convention
; NUM  = rax
; ARG0 = rdi
; ARG1 = rsi
; ARG2 = rdx
; ARG3 = r10
; ARG4 = r8
; ARG5 = r9
; RET  = rax

; This is the kernel entrypoint for system calls.
; arch_stage_2() puts his address in the LSTAR MSR.
global syscall_entry
syscall_entry:
  swapgs              ; load kernel GS 
  mov [gs:24], rsp    ; save userspace rsp 
  mov rsp, [gs:16]    ; rsp = cpu->proc 
  mov rsp, [rsp+8]    ; rsp = cpu->proc->kstack
  add rsp, 4096       ; rsp += KSTACK_SIZE
  
  ; save general purpose registers
  push rax
  push rbx
  push rcx
  push rdx
  push rbp
  push rsi
  push rdi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  
  mov rdi, rsp
  call syscall_dispatcher

  ; restore general purpose registers
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

  mov rsp, [gs:24]    ; restore userspace rsp 
  swapgs              ; restore userspace GS
  o64 sysret          ; fast return from syscall
