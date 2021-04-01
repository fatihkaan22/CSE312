.text

main:

	li, $s0, 1							# i

	li $v0, 5
	syscall 								# read number
	move $s1, $v0

L1: 
	div $s1, $s0
	mfhi $t1 								# t1 = a0 % t0
	bne $t1, $zero, continue

	li $v0,1								# print integer
	move $a0, $s0
	syscall
	li $v0, 4								# print " "
	la $a0, space
	syscall

continue:
	addi $s0, $s0, 1 				# i++
	ble $s0,$s1, L1

	j exit

exit:
	li $v0, 10							# system call to exit program
	syscall

.data
space:	.asciiz " "
