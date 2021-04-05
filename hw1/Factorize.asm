.text

main:

	# print "Enter number: "
	li $v0, 4								
	la $a0, str_enter
	syscall

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
	la $a0, str_space
	syscall

continue:
	addi $s0, $s0, 1 				# i++
	ble $s0,$s1, L1

	# print new line
	li $v0, 4								
	la $a0, str_newline
	syscall

	j exit

exit:
	li $v0, 10							# system call to exit program
	syscall

.data
str_space:	.asciiz " "
str_newline:	.asciiz "\n"
str_enter: .asciiz "Enter number: "
