        addi x1, x0, 1
        addi x2, x0, 10
        addi x3, x0, 1
loop:
        addi x3, x3, 1
        sw x3, 0(x0)
        add x0, x0, x0
        lw x1, 0(x0)
        blt x1, x2, loop

