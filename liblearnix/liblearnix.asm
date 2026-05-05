BITS 64

section .text

; the _start routine is responsible for initializing the
; user stack and call main() with the correct arguments
extern main
global _start
_start:
  xor rbp, rbp  
  xor rdi, rdi  ; ignore argc
  xor rsi, rsi  ; ignore argv
  
  ; call C code's main() function
  call main

  ; exit() when main returns
  mov rdi, rax
  mov rax, 60
  syscall

; This assembly stub is called by learnixlibc whenever a systemcall needs to happen
; RDI = syscall number
; RSI = arg0
; RDX = arg1
; RCX = arg2
; R8 = arg3
; R9 = arg4
global _do_syscall
_do_syscall:
  mov rax, rdi  ; syscall number
  mov rdi, rsi  ; arg0
  mov rsi, rdx  ; arg1
  mov rdx, rcx  ; arg2
  mov r10, r8   ; arg3
  mov r8, r9    ; arg4

  syscall

  ret
