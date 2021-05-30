.data
mutex:     .space 1
count:     .word 0      # current size of the queue
size:      .word 50     # capacity of the queue
produced:  .word 0      # number of item produced
consumed:  .word 0      # number of item consumed

producer_str:   .asciiz "producer "
consumer_str:   .asciiz "consumer "
newline:        .asciiz "\n"
produced_str:   .asciiz "total produced: "
consumed_str:   .asciiz "total consumed: "

.text
.globl main

main:

    li $v0, 21     # mutex init
    la $a0, mutex
		syscall

    li $t1, 0

    li $v0, 18      # thread create
    la $s0, producer
    syscall
    move $s1, $v0


    li $v0, 18      # thread create
    la $s0, consumer
    syscall
    move $s2, $v0

    li $v0, 19      # thread join
    move $a0, $s1   # wait for producer thread
    syscall

    li $v0, 19      # thread join
    move $a0, $s2   # wait for consumer thread
    syscall

    # print total produced
    li $v0, 4
    la $a0, produced_str
    syscall
    li $v0, 1
    la $t0, produced
    lw $a0, ($t0)
    li $v0, 1
    syscall

    jal print_newline

    # print total consumed
    li $v0, 4
    la $a0, consumed_str
    syscall
    li $v0, 1
    la $t0, consumed
    lw $a0, ($t0)
    li $v0, 1
    syscall

    jal print_newline

    li $v0, 10
    syscall

producer:
    nop

    li $s1, 0
produce_loop:
    jal wait_empty
    jal mutex_lock

    li $v0, 4
    la $a0, producer_str
    syscall

    jal produce
    jal print_newline

    jal mutex_unlock

    addi $s1, $s1, 1
    blt $s1, 10000, produce_loop
    li $v0, 20      # thread exit
    syscall

consumer:
    nop

    li $s1, 0
consume_loop:
    jal wait_full
    jal mutex_lock

    li $v0, 4
    la $a0, consumer_str
    syscall

    jal consume
    jal print_newline

    jal mutex_unlock

    addi $s1, $s1, 1
    blt $s1, 10000, consume_loop


    li $v0, 20      # thread exit
    syscall

produce:
    la $t0, count
    lw $t1, ($t0)
    addi $t1, $t1, 1
    li $v0, 1
    move $a0, $t1
    syscall
    sw $t1, ($t0)

    la $t0, produced
    lw $t1, ($t0)
    addi $t1, $t1, 1
    sw $t1, ($t0)
    jr $ra

consume:
    la $t0, count
    lw $t1, ($t0)
    addi $t1, $t1, -1
    li $v0, 1
    move $a0, $t1
    syscall
    sw $t1, ($t0)

    la $t0, consumed
    lw $t1, ($t0)
    addi $t1, $t1, 1
    sw $t1, ($t0)
    jr $ra

print_newline:
    li $v0, 4
    la $a0, newline
    syscall
    jr $ra

wait_empty:
    la $t0, count
    lw $t1, ($t0)
    la $t0, size
    lw $t2, ($t0)
    blt $t1, $t2, r_wait_empty
    j wait_empty
r_wait_empty:
    jr $ra
    
wait_full:
    la $t0, count
    lw $t1, ($t0)
    bne $t1, $zero, r_wait_full
    j wait_full
r_wait_full:
    jr $ra

mutex_lock:
    li $v0, 22
    la $a0, mutex
    syscall
    jr $ra

mutex_unlock:
    li $v0, 23
    la $a0, mutex
    syscall
    jr $ra
