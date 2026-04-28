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

; when handling an ISR only the first part of the stub depends
; on the vector number, but after we've called intr_dispatcher()
; it's the same
%macro isr_common 0
  ; were we in unserspace?
  cmp qword [rsp+24], 0x08
  je %%from_kern
  swapgs                  ; YES, then load kernel' GS

%%from_kern               ; jump here if the IRQ was generated in ring 0
  push_regs

  mov rdi, rsp
  call intr_dispatcher

  jmp kernel_exit
%endmacro

%macro isr_noerr 1 
isr_stub_%+%1:
  push qword 0      ; dummy error code
  push qword %1     ; vector number
  isr_common
%endmacro

%macro isr_err 1 
isr_stub_%+%1:
  push qword %1     ; vector number
  isr_common
%endmacro

extern kernel_exit
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
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31
isr_noerr 32    ; ISR0: hw timer
isr_noerr 33    ; ISR1: PS/2 keyboard

; stub table
section .rodata
GLOBAL isr_stubs_table
isr_stubs_table:
%assign i 0
%rep 34
  dq isr_stub_%+i
%assign i i+1
%endrep
