.text

main:

	# print "size: "
	li $v0, 4								
	la $a0, str_size
	syscall

	# read size
	li $v0, 5
	syscall
	move $s3, $v0

	# print "enter: "
	li $v0, 4								
	la $a0, str_enter
	syscall

	# get integers
	la $t5, arr
	li $t0, 0
readLoop:
	bge $t0, $s3, readLoopExit
	li $v0, 5
	syscall
	sw $v0, 0($t5)
	addi $t5, $t5, 4
	addi $t0, $t0, 1
	j readLoop
readLoopExit:

#	li $s3, 8								# size
	addi $t0, $s3 -1				# size - 2

L1:
	li $t2, 0							# changed = false 
	li $t3, 0							# i = 0
	la $t5, arr
L2:
	bge $t3, $t0, L2Exit

	lw $s5, 0($t5) 					# *ptr
	lw $s6, 4($t5)  				# *(ptr + 1)
	ble $s5, $s6, ifExit 		# if ( *ptr > * (ptr + 1) )
	sw $s6, 0($t5) 					# ptr = ptr + 1
	sw $s5, 4($t5) 					# ptr + 1 = ptr
	li, $t2, 1							# changed = true 
ifExit:
	addi $t3, $t3, 1 				# ++i
	addi $t5, $t5, 4 				# ++i
	j L2
L2Exit:

	bne $t2, $zero, L1 				# while(changed)

	la $a0, arr 					# arr
	move $a1, $s3 					# size
	jal printArr 						# print(arr, size)

	# print new line
	li $v0, 4								
	la $a0, str_newline
	syscall

	j exit

exit:
	li $v0, 10							# system call to exit program
	syscall

# a0: arr, a1: size
printArr:
	move $t2, $a0 					# t2 = arr
	move $t1, $a1 					# t1 = size
printLoop:
	ble $t1, $zero, ret_printArr
	lw $a0, 0($t2)
	li $v0, 1
	syscall
	la $a0, str_space
	li $v0, 4
	syscall
	addi $t2, $t2, 4
	addi $t1, $t1, -1
	j printLoop
ret_printArr:
	jr $ra



.data

test1:	.word	41 67 34 0 69 24 78 58 	
test2:	.word	62 64 5 45 81 27 61 91	
test3:	.word	95 42 27 36 91 4 2 53	
test4:	.word	92 82 21 16 18 95 47 26	
test5:	.word	71 38 69 12 67 99 35 94	 
test6:	.word	11 22 33 73 64 41 11	 

arr:	.space	400					# MAX_SIZE=100

str_space:	.asciiz " "
str_size:	.asciiz "Enter number of integers: "
str_enter:	.asciiz "Enter integers: "
str_newline: .asciiz "\n"
