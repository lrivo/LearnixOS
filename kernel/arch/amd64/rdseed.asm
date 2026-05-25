BITS 64
section .text

; rdi: uint8_t *vec
; rsi: size_t n
global arch_rand_bytes
arch_rand_bytes:
  ; rax: RDSEED ouput
  ; rcx: RDSEED number of reties
  
  ; software must check if RDSEED has failed
  ; we allow 100 tries before giving up
  mov rcx, 100
.retry:
  rdrand rax    ; generate a 64 bit random number
  jc .done      ; if CF=1 we have a valid number
  test rcx, 0   ; if CF=0 check if we have tries left 
  je .fail
  sub rcx, 1
.fail:
  mov rax, -1   ; return error to caller
  ret
.done:
  ; TODO: byte by byte copy into rdi
  mov [rdi], rax
  mov rax, 0
  ret
