  .text
  .align 2

.global main0
main0:
    addi sp, sp, -16 # 分配栈帧
    sw ra, 12(sp)    # 保存返回地址

    li a0, 1
    li a1, 1

    call foo

    lw ra, 12(sp)   # 恢复返回地址
    addi sp, sp, 16 # 释放栈帧
    # 装入返回值！
    ret

.global foo2
foo2:
    addi sp, sp, -16
    sw ra, 12(sp)

    lw ra, 12(sp)
    addi sp, sp, 16
    ret

.global foo
foo:
    addi sp, sp, -16
    sw ra, 12(sp)

    call foo2

    sw a0, 8(sp)

    call foo2

    add a0, a0, a1
    lw a2, 8(sp)
    add a2, a2, a1
    add a2, a2, a0


    lw ra, 12(sp)   # 恢复返回地址
    addi sp, sp, 16 # 释放栈帧
    mv a0, a2       # 装入返回值
    ret

