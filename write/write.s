.global _main
.align 2

_main:
  adr x0, filename
  mov x1, 0x0601
  mov x2, 0644
  mov x16, #5
  svc #0
  mov x9, x0

  adr x1, text
  mov x2, #30
  mov x16, #4
  svc #0

  mov x0, x9
  mov x16, #6
  svc #0

  mov x0, #0
  mov x16, #1
  svc #0

text:
  .ascii "now this is written into file\n"

filename:
  .asciz "output.txt"
