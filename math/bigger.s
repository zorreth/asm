.global _main
.align 2

_main:
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  mov x1, #50
  mov x2, #10

  cmp x1, x2
  b.lt number_two_is_bigger

  mov x0, x1

  b print_result

number_two_is_bigger:
  mov x0, x2

print_result:
  str x0, [sp, #-16]!
  adrp x0, format_str@PAGE
  add x0, x0, format_str@PAGEOFF
  bl _printf

  add sp, sp, #16

  mov x0, #0
  ldp x29, x30, [sp], #16
  ret

format_str:
  .asciz "The bigger number is: %d\n"
