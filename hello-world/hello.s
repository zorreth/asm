.global _main
.align 2

_main:
  mov x0, #1
  adr x1, msg
  mov x2, #14
  mov x16, #4
  svc #0

  mov x0, #0
  mov x16, #1
  svc #0

msg:
  .ascii "Hello, World!\n"
