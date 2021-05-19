.data
msg:   .asciiz "m"
t1:   .asciiz "1"
t2:   .asciiz "2"

.text
.globl main

main:
        li $v0, 18      # thread create
        la $a0, thread1
        syscall

        li $v0, 18      # thread create
        la $a0, thread2
        syscall

#        li $v0, 19      # thread join
#        syscall

l1:
        li $v0, 4       # syscall 4 (print_str)
        la $a0, msg     # argument: string
        syscall         # print the string
	      j l1
        
        jr $ra          # retrun to caller


thread1: 
        li $v0, 4       # syscall 4 (print_str)
        la $a0, t1      # argument: string
        syscall         # print the string
	     	j thread1
        
        jr $ra



thread2: 
        li $v0, 4       # syscall 4 (print_str)
        la $a0, t2      # argument: string
        syscall         # print the string
	      j thread2
        
        jr $ra

