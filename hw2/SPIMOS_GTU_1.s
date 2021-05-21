.data
msg:   .asciiz " tm "
t1:   .asciiz " t1 "
t2:   .asciiz " t2 "
t3:   .asciiz " t3 "
mutex1: .space 1
var:    .word 0

.text
.globl main


main:
    li $v0, 18      # thread create
    la $a0, thread1
    syscall

#    li $v0, 18      # thread create
#    la $a0, thread2
#    syscall

#    li $v0, 18      # thread create
#    la $a0, thread3
#    syscall


#    li $a0, 1       # a0: thread id
#    li $v0, 19      # join thread id with 1
#    syscall


# mutex init
    li $v0, 21
    la $a0, mutex1
    syscall

# mutex lock
    la $a0, mutex1
    li $v0, 22
    syscall

# BEGIN: critical 
	  la $t0, var
	  lw $t1, ($t0)
l0:
    la $a0, msg     # argument: string
    li $v0, 4       # syscall 4 (print_str)
    syscall
    jal inc
    blt $t1, 1000, l0
	  sw $t1, ($t0)
# END: critical 

# mutex unlock
    la $a0, mutex1
    li $v0, 23
    syscall

    li $v0, 20     # thread exit
    syscall


inc:
    li $v0, 1
	  move $a0, $t1
    syscall
    addi $t1, $t1, 1
    jr $ra

thread1:
    nop

# mutex lock
    la $a0, mutex1
    li $v0, 22
    syscall

# BEGIN: critical 
	  la $t0, var
	  lw $t1, ($t0)
l1:
    la $a0, t1     # argument: string
    li $v0, 4       # syscall 4 (print_str)
    syscall
    jal inc
    blt $t1, 100, l1
	  sw $t1, ($t0)
# END: critical 

# mutex unlock
    la $a0, mutex1
    li $v0, 23
    syscall

    li $v0, 20     # thread exit
    syscall

thread2:
    nop
    la $a0, t2     # argument: string
    li $v0, 4       # syscall 4 (print_str)
    syscall
	  la $t0, var
	  lw $t1, ($t0)
    jal inc
    blt $t1, 100, thread2

    li $v0, 20     # thread exit
    syscall
