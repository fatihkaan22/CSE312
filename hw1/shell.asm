print_string:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li $v0, 4
        la $a0,($4)
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
read_string:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        sw      $5,12($fp)
        li $v0, 8
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
create_process:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li $v0, 18
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        j       $31
.data
$LC0:
        .asciiz  "run \000"
$LC1:
        .asciiz  "\000\000"
				.text
is_run:
        addiu   $sp,$sp,-32
        sw      $fp,28($sp)
        move    $fp,$sp
        sw      $4,32($fp)
        la      $2,$LC0
        sw      $2,8($fp)
        lw      $2,32($fp)
        sw      $2,12($fp)
        b       $L5
$L8:
        lw      $2,12($fp)
        lb      $3,0($2)
        lw      $2,8($fp)
        lb      $2,0($2)
        beq     $3,$2,$L6
        la      $2,$LC1
        b       $L7
$L6:
        lw      $2,8($fp)
        addiu   $2,$2,1
        sw      $2,8($fp)
        lw      $2,12($fp)
        addiu   $2,$2,1
        sw      $2,12($fp)
$L5:
        lw      $2,8($fp)
        lb      $2,0($2)
        bne     $2,$0,$L8
        lw      $2,12($fp)
        sw      $2,16($fp)
        b       $L9
$L10:
        lw      $2,16($fp)
        addiu   $2,$2,1
        sw      $2,16($fp)
$L9:
        lw      $2,16($fp)
        lb      $3,0($2)
        li      $2,10                 # 0xa
        bne     $3,$2,$L10
        lw      $2,16($fp)
        sb      $0,0($2)
        lw      $2,12($fp)
$L7:
        move    $sp,$fp
        lw      $fp,28($sp)
        addiu   $sp,$sp,32
        j       $31
.data
$LC2:
        .asciiz  "exit\012\000"
				.text
is_exit:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        la      $2,$LC2
        sw      $2,8($fp)
        lw      $2,24($fp)
        sw      $2,12($fp)
        b       $L12
$L15:
        lw      $2,12($fp)
        lb      $3,0($2)
        lw      $2,8($fp)
        lb      $2,0($2)
        beq     $3,$2,$L13
        move    $2,$0
        b       $L14
$L13:
        lw      $2,8($fp)
        addiu   $2,$2,1
        sw      $2,8($fp)
        lw      $2,12($fp)
        addiu   $2,$2,1
        sw      $2,12($fp)
$L12:
        lw      $2,12($fp)
        lb      $2,0($2)
        bne     $2,$0,$L15
        li      $2,1                        # 0x1
$L14:
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        j       $31
.data
$LC3:
        .asciiz  "[shell]> \000"
$LC4:
        .asciiz  "ERROR: command not found\012\000"
				.text
main:
        addiu   $sp,$sp,-296
        sw      $31,292($sp)
        sw      $fp,288($sp)
        move    $fp,$sp
        sw      $4,296($fp)
        sw      $5,300($fp)
$L21:
        la      $4,$LC3
        jal     print_string
        addiu   $2,$fp,28
        li      $5,256                  # 0x100
        move    $4,$2
        jal     read_string
        addiu   $2,$fp,28
        move    $4,$2
        jal     is_exit
        bne     $2,$0,$L24
        addiu   $2,$fp,28
        move    $4,$2
        jal     is_run
        sw      $2,24($fp)
        lw      $2,24($fp)
        lb      $2,0($2)
        beq     $2,$0,$L19
        lw      $4,24($fp)
        jal     create_process
        b       $L21
$L19:
        la      $4,$LC4
        jal     print_string
        b       $L21
$L24:
        nop
        move    $2,$0
        move    $sp,$fp
        lw      $31,292($sp)
        lw      $fp,288($sp)
        addiu   $sp,$sp,296
        j       $31
