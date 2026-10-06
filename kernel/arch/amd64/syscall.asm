BITS 64
section .text

extern kernel_exit
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

  ; construct the x86_64 struct intr_trap_frame
  push qword 0x1B     ; GDT_UDATA | 3
  push qword [gs:24]  ; userspace RSP
  push r11            ; rflags
  push qword 0x23     ; GDT_UCODE | 3
  push rcx            ; userspace RIP

  ; push dummy error code and vector_num
  push qword 0
  push qword 0

  ; general purpose registers (rcx and r11 are meaningless here)
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

  ; go through the common kernel_exit
  jmp kernel_exit
