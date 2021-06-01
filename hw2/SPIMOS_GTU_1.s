.data
space:     .asciiz " "
tmp:       .word 0:128   # temproary array for merge
# array with size 16 
array:     .word  1 8 14 10 11 2 4 3 12 13 9 15 7 16 6 5
# array with size 32 
#array:   .word  26 8 1 16 11 3 13 5 4 20 32 7 15 28 9 30 17 23 25 10 14 2 27 21 22 12 19 18 6 29 31 24

.text
main:
	la $a0, array
	li $a1, 0       # low
	li $a2, 7       # high
	li $v0, 18      # thread create
	la $s0, thread_mergesort
	syscall
	move $s1, $v0   # obtain thread id

	li $a1, 8       # low
	li $a2, 15      # high
	li $v0, 18      # thread create
	la $s0, thread_mergesort
	syscall
	move $s2, $v0   # obtain thread id

	li $v0, 19      # thread 1 join
	move $a0, $s1
	syscall

	li $v0, 19      # thread 2 join
	move $a0, $s2
	syscall

	# merge 2 parts of the partially sorted array by threads
	la $a0, array# load address of array to $a0 as an argument
	li, $a1, 0 # low
	li, $a3, 7 # mid
	li, $a2, 15  # high
	jal merge

	jal print
	li $v0, 10     # exit
	syscall

thread_mergesort:
	jal mergesort
	li $v0, 20     # thread exit
	syscall

mergesort: 
	# base case: if (low >= high) return
	bge $a1, $a2, return_mergesort

	# sort first half
	addi, $sp, $sp, -16
	sw, $ra, 12($sp)
	sw, $a1, 8($sp)
	sw, $a2, 4($sp)

	add $s0, $a1, $a2  # mid = low + high
	sra $s0, $s0, 1    # mid = (low + high) / 2
	sw $s0, ($sp)

	move $a2, $s0      # high = mid
	jal mergesort

	# sort second half
	lw $s0, ($sp)
	addi $a1, $s0, 1
	lw $a2, 4($sp)
	jal mergesort
	
	# merge
	lw, $a1, 8($sp)
	lw, $a2, 4($sp)
	lw, $a3, 0($sp)
	jal merge

	# restore stack
	lw $ra, 12($sp)
	addi $sp, $sp, 16

return_mergesort:
	jr $ra # return to calling routine
	
merge:
	move $s0, $a1     # low
	move $s1, $a1     # low (index of tmp)
	addi $s2, $a3, 1   # mid + 1

merge_loop: 
	blt $a3,  $s0, remaining_left # if (mid < i) next While loop
	blt $a2,  $s2, remaining_left # if (high < j) next While loop
	# if (i <= mid && j <=high)
	sll $t0, $s0, 2
	add $t0, $t0, $a0
	lw $t1, ($t0)
	sll $t2, $s2, 2
	add $t2, $t2, $a0
	lw $t3, ($t2)
	blt $t3, $t1, right_arr  # if (arr[j] < arr[i]) get from right array
	# get from left array
	sll $t4, $s1, 2
	la $t5, tmp
	add $t5, $t5, $t4
	sw $t1, 0($t5)
	addi $s0, $s0, 1
	addi $s1, $s1, 1
	j merge_loop
	
right_arr:
	sll $t1, $s2, 2
	add $t1, $t1, $a0
	lw $t2, ($t1)
	la $t3, tmp
	sll $t4, $s1, 2
	add $t3, $t3, $t4
	sw $t2, ($t3)
	addi $s1, $s1, 1
	addi $s2, $s2, 1
	j merge_loop
	
remaining_left:
	blt $a3, $s0, remaining_right
	sll $t0, $s0, 2
	add $t0, $a0, $t0
	lw $t1, ($t0)
	la $t2, tmp
	sll $t3, $s1, 2
	add $t3, $t3, $t2
	sw $t1, ($t3)
	addi $s1, $s1, 1
	addi $s0, $s0, 1
	j remaining_left
	
remaining_right:
	blt $a2, $s1, tmp_to_array
	sll $t1, $s2, 2
	add $t1, $t1, $a0
	lw $t2, ($t1)
	la $t3, tmp
	sll $t4, $s1, 2
	add $t3, $t3, $t4
	sw $t2, ($t3)
	addi $s1, $s1, 1
	addi $s2, $s2, 1
	j remaining_right

tmp_to_array:       # copy tmp to original array
	move $t0, $a1
	addi $t1, $a2, 1  # high + 1
	la $t4, tmp
copy_loop:
	bge $t0, $t1 return_merge
	sll $t2, $t0, 2
	add $t3, $t2, $a0
	add $t5, $t2, $t4
	lw $t6, 0($t5)
	sw $t6, 0($t3)
	addi $t0, $t0, 1
	j copy_loop

return_merge:
	jr $ra

print:
	move $t0, $a1 # idx
	move $t1, $a2 # size
	la $t4, array
	
print_loop:
	sll $t3, $t0, 2
	add $t3, $t3, $t4
	lw $a0, ($t3)
	li $v0, 1
	syscall
	addi $t0, $t0, 1
	la $a0, space
	li $v0, 4
	syscall
	bge $t1, $t0, print_loop
	jr $ra
