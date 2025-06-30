.globl _start
.text
_start:
        mov     ax, 1024
        mov     ax, #1024
        mov     bx, ax
        mov     ax, *-8(bx)
        mov     bp, bx
        mov     ax, *-10(bp)
        ret
        .data
fmt:    .byte 37, 100, 10, 0
