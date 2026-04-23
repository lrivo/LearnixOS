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
  mov rsp, [rsp+8]   ; rsp = cpu->proc->kstack
  add rsp, 4096       ; rsp += KSTACK_SIZE
  
  push r11            ; save userspace RFLAGS
  push rcx            ; save userspace RIP
  
  ; we need to translate the syscall calling convention (above)
  ; to x86_64 C calling convention
  ; r9 is first because the 6th C parameter must be passed on the stack
  push r9
  push rax
  push rdi
  push rsi
  push rdx
  push r10
  push r8
  pop r9 
  pop r8
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  ; at this point the C function has num-arg4 on
  ; rdi-rsi-rdx-rcx-r8-r9 and arg5 on the stack
  call syscall_dispatcher
  
  pop r9              ; the orignal arg5 is still on the stack
  pop rcx             ; restore userspace RIP
  pop r11             ; restore userspace RFLAGS

  mov rsp, [gs:24]    ; restore userspace rsp 
  swapgs              ; restore userspace GS
  o64 sysret          ; fast return from syscall
