BITS 64

SECTION .text

GLOBAL reloadSegments
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

