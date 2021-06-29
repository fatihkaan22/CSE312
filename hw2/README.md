# How to run?

1. Copy `syscall.cpp` and `syscall.h` into `CPU` folder.
2.  `make && ./spim -file /path/to/SPIMOS_GTU_X.s` 

## SPIMOS_GTU_1 (Producer & Consumer)

`./spim -file /path/to/SPIMOS_GTU_1.s` 

- Producer & consumer implementation in MIPS assembly.
- 1 producer and 1 consumer threads are created.
- Queue size is 50, each thread is going to produce/consume 1000 items.
- Mutexes and busy waiting are used to solve synchronization problmes.

## SPIMOS_GTU_2 (Multithread Mergesort)

`./spim -file /path/to/SPIMOS_GTU_2.s` 

- Multithread mergesort implementation in MIPS assembly. 
- 2 threads are created with array size 16.

# Notes

- source files of the spim have taken from r739 commit.
