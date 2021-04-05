.text

main:

	li, $s0, 0							# i
	li, $s1, 1000						# n

L1: 
	li $v0,1								# print integer
	move $a0, $s0
	syscall
	jal isPrime
	addi $s0, $s0, 1 				# i++
	ble $s0,$s1, L1

	j exit

isPrime:
	blt $a0, 2, isPrimeEnd 	# if < 0 return false
	beq $a0, 2, printPrime 	# if == 2 return true

	li $t0, 2 
L2:
	bge $t0, $a0, printPrime
	div $a0, $t0
	mfhi $t1 								# t1 = a0 % t0
	beq $t1, $zero, isPrimeEnd
	addi $t0, $t0, 1				# ++i
	j L2

printPrime:
	li $v0, 4
	la $a0, prime
	syscall
isPrimeEnd:
	li $v0, 4
	la $a0, newline
	syscall
	jr $ra

exit:
	li $v0, 10							# system call to exit program
	syscall
	
.data
arr:	.space	100
prime:	.asciiz " prime"
newline:	.asciiz "\n"
