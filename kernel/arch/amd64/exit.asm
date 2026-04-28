BITS 64

; This file contains the kernel_exit routine.
; It is the common exit point for both interrupts and syscalls.
; Both interrupts and syscalls constructs the same stack frame,
; defined by struct intr_trap_frame.
; Using iretq we can return both from an interrupt and a syscall.

extern schedule

global kernel_exit
kernel_exit:
  ; NOTE: at this point rsp points at r15, the last pushed register
  
  ; check if we need to reschedule
  cmp byte [gs:8], 0      ; is cpu->proc_need_resched == 0?
  jz no_resched           ; YES, immediately return
  mov byte [gs:8], 0      ; NO, clear cpu->proc_need_resched
  call schedule           ; call the scheduler's entry point

no_resched:
  ; restore the general purpose registers
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
  
  add rsp, 16             ; skip error and vector_num
  test [rsp+8], 3         ; CS & 3 (are we returning to userspace?)
  jz ret_kern             ; NO, we're returning in kernel mode
  swapgs                  ; restore userspace GS

ret_kern:
  iretq
