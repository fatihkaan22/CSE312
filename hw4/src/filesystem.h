#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

#define NO_BLOCKS 4096 // 2^12 possible addresses

enum BlockSizePref { BS_512 ,BS_1024, BS_2048, BS_4096 };



/* const static uint32_t INODES_PER_BLOCK = 128; */
/* const static uint32_t POINTERS_PER_INODE = 5; */
/* const static uint32_t POINTERS_PER_BLOCK = 1024; */

struct super_block {
	uint32_t block_size;
	uint32_t block_count;
// inode count
// free blocks
// free inodes
// first block
};

struct entry {
	// name 
	unsigned address : 12; // FAT-12
};

/* struct inode { */
/* 	// pointers */
/* }; */

// free bitmap



#endif /* end of include guard: FILESYSTEM_H */
