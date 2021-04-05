# Homework 1

- https://godbolt.org/  MIPS gcc 5.4 compiler used in order to obtain assembly file.
- `-mno-explicit-relocs` compiler flag is used, since SPIM doesn't allow %hi and %lo registers.
- Comments and unused lables are filtered from the output.
- Directives filtered from output. `.data` and `.text` segments added to assembly file, in order to avoid unsupported directives.
- `syscall.cpp` and `syscall.h` changed, so that it can handle `create_process` syscall. It saves the state of the program, runs desired asm file and resotres the state back.

Shell will print the prompt and waits for user to write the filename to run.

```
[shell]> run <filename.asm>
```

## ShowPrimes.asm

```
[shell]> run ShowPrimes.asm
0
1
2 prime
3 prime
4
5 prime
...
996
997 prime
998
999
1000
```

## Factorize.asm


```
[shell]> run Factorize.asm
Enter number: 20
1 2 4 5 10 20
```

## BubbleSort.asm

```
[shell]> run BubbleSort.asm
```

## exit

```
[shell]> exit
```

## Error Handling

```
[shell]> foo
ERROR: command not found
```
