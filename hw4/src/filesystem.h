#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define NO_BLOCKS 4096 // 2^12 possible addresses

enum BlockSizePref { BS_512, BS_1024, BS_2048, BS_4096 };

enum { BITS_PER_WORD = sizeof(uint32_t) * CHAR_BIT }; // 32
// array is uint32_t: 2^12 bits = 512 bytes = 128 uint32
enum { BITMAP_WORD_SIZE = NO_BLOCKS / BITS_PER_WORD };           // 128
enum { BITMAP_BYTE_SIZE = sizeof(uint32_t) * BITMAP_WORD_SIZE }; // 512

enum { FAT_TABLE_BYTE_SIZE = (NO_BLOCKS * 12) / CHAR_BIT }; // 6 * 1024
enum {
  FAT_TABLE_WORD_SIZE = FAT_TABLE_BYTE_SIZE / sizeof(uint32_t)
}; // 6 * 1024
enum { NO_ROOT_BLOCKS = 14 };
enum { YEAR_START = 1900 };

#define ATTR_DIR 0
#define ATTR_FILE 1

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

struct super_block {
  uint32_t block_size;
  uint32_t block_count;
  uint32_t free_bitmap_start;
  uint32_t free_block_size;
  uint32_t no_free_blocks;
  uint32_t fat_table_start;
  uint32_t fat_table_block_size;
  uint32_t root_start;
  uint32_t root_size;
  uint32_t data_start;
  uint32_t data_size;
};

struct dir_entry {
  char name[20];           // 20 bytes
  uint16_t address;        // 2 bytes FAT-12 (used only least significant 12 bits)
  uint8_t attr;            // 1 byte
  uint8_t size[3];         // 3 bytes : max 16 MB size
  uint8_t time[3];         // 3 bytes : HMS
  uint8_t date[3];         // 3 bytes : DMY (Y: 1900+)
} __attribute__((packed)); // required for struct to be exactly 32 bytes

typedef struct dir_entry dir_entry;

struct uint12 {
  uint16_t x : 12;
};

typedef struct uint12 uint12;

extern FILE *fp;
extern struct super_block super_blk;
extern uint32_t free_bitmap[BITMAP_WORD_SIZE];
extern uint12 fat_table[NO_BLOCKS];

// free bitmap

void set_bit(uint32_t *arr, int n);
void clear_bit(uint32_t *arr, int n);
int get_bit(uint32_t *arr, int n);
void print_bitmap(uint32_t *free_bitmap);

void write_zeros(int bytes);
void write_superblock();
void write_bitmap();
void write_fattable();

#endif /* end of include guard: FILESYSTEM_H */
