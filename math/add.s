.global _main
.align 2

_main:
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  mov x1, #50
  mov x2, #10
  bl add_numbers

  stp x1, x2, [sp, #-32]!

  str x0, [sp, #16]
  adrp x0, format_str@PAGE
  add x0, x0, format_str@PAGEOFF
  bl _printf

  add sp, sp, #32

  mov x0, #0
  ldp x29, x30, [sp], #16
  ret

add_numbers:
  add x0, x1, x2
  ret

format_str:
  .asciz "The result of %d + %d is: %d\n"
