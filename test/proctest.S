
    .global _start
_start:
    li a7, 0
    ecall
    ecall
halt:
    j halt
