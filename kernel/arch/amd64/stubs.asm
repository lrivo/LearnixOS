BITS 64

section .text

; ==== GDT ====
global reloadSegments
reloadSegments:
   ; Reload CS register:
   push 0x8                     ; Push code segment to stack, 0x08 is a stand-in for your code segment
   lea rax, [rel .reload_CS]    ; Load address of .reload_CS into RAX
   push rax                     ; Push this value to the stack
   retfq                        ; Perform a far return
.reload_CS:
   ; Reload data segment registers
   mov    ax, 0x10
   mov    ds, ax
   mov    es, ax
   mov    fs, ax
   mov    gs, ax
   mov    ss, ax
   ret

; ==== INTERRUPTS ====
%macro push_regs 0
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
%endmacro

%macro pop_regs 0 
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
%endmacro

%macro isr_noerr 1 
isr_stub_%+%1:
  ; dummy error code
  push 0

  ; vector number
  push %1
  
  ; save registers
  push_regs
  
  ; pass the current stack frame to intr_dispatcher
  mov rdi, rsp

  ; call C dispatcher
  cld
  call intr_dispatcher

  ; restore registers
  pop_regs
  
  ; removes vector number and dummy error code
  add rsp, 16

  ; return from interrupt
  iretq
%endmacro

%macro isr_err 1 
isr_stub_%+%1:
  ; vector number
  push %1

  ; save registers
  push_regs
  
  ; pass the current stack frame to intr_dispatcher
  mov rdi, rsp

  ; call C dispatcher
  cld
  call intr_dispatcher

  ; restore registers
  pop_regs

  ; removes vector number and error code
  add rsp, 16

  ; return from interrupt
  iretq
%endmacro

extern intr_dispatcher
isr_noerr 0       ; Division by Zero
isr_noerr 1       ; Debug Exception
isr_noerr 2       ; NMI Interrupt (Non Maskable)
isr_noerr 3       ; Breakpoint
isr_noerr 4       ; Overflow
isr_noerr 5       ; BOUND Range Exceeded
isr_noerr 6       ; Invalid Opcode
isr_noerr 7       ; Device Not Available (No Math Coprocessor)
isr_err 8         ; Double Fault
isr_noerr 9       ; Coprocessor Segment Overrun
isr_err 10        ; Invalid TSS
isr_err 11        ; Segment Not Present
isr_err 12        ; Stack-Segment Fault
isr_err 13        ; General Protection Fault
isr_err 14        ; Page Fault
isr_noerr 15      ; RESERVED
isr_noerr 16      ; x87 FPU Error
isr_err 17        ; Alignment Check
isr_noerr 18      ; Machine Check
isr_noerr 19      ; SIMD Floating-Point Exception
isr_noerr 20      ; Virtualization Exception
isr_err 21        ; Control Protection Exception


; stub table
section .rodata
GLOBAL isr_stubs_table
isr_stubs_table:
%assign i 0
%rep 22
  dq isr_stub_%+i
%assign i i+1
%endrep
