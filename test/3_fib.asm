        addi x1, x0, 100
        addi x2, x0, 1
        addi x3, x0, 1
        addi x4, x0, 0
        addi x5, x0, 0
loop:
        sw x2, 0(x5)
        sw x3, 4(x5)
        add x2, x2, x3
        add x3, x2, x3
        addi x4, x4, 1
        addi x5, x5, 8
        blt x4, x1, loop
