.section .text
.global _start
_start:
  li a0, 0
  li a7, 93    # exit syscall
  ecall

.global main
main:

