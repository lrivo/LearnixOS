BITS 64
section .text

; rdi: uint64_t *out 
global rdseed_u64 
rdseed_u64:
  ; rax: RDSEED ouput
  ; rcx: RDSEED number of reties
  
  ; software must check if RDSEED has failed
  ; we allow 100 tries before giving up
  mov rcx, 100
.retry:
  rdseed rax    ; generate a 64 bit random number
  jc .done      ; if CF=1 we have a valid number
  dec rcx       ; rcx -= 1
  jz .fail      ; if rcx hits 0 we've failed
  pause
  jmp .retry    ; otherwise we can retry
.fail:
  mov rax, -1   ; return error to caller
  ret
.done:
  mov [rdi], rax  ; copy rdseed's output
  mov rax, 0      ; no error
  ret
