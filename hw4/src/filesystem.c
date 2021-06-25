#include "filesystem.h"
#include <stdlib.h>

FILE *fp;
struct super_block super_blk;
uint32_t free_bitmap[BITMAP_WORD_SIZE];
uint12 fat_table[NO_BLOCKS];

void print_bitmap(uint32_t *free_bitmap) {
  printf("====== BITMAP ======");
  for (int i = 0; i < NO_BLOCKS; ++i) {
    if (i % 64 == 0) {
      puts("");
      printf("%5d :", i);
    }
    if (i % 16 == 0)
      printf(" ");
    printf("%d", get_bit(free_bitmap, i));
  }
  puts("");
}

void set_bit(uint32_t *arr, int n) {
  arr[WORD_OFFSET(n)] |= ((uint32_t)1 << BIT_OFFSET(n));
}

void clear_bit(uint32_t *arr, int n) {
  arr[WORD_OFFSET(n)] &= ~((uint32_t)1 << BIT_OFFSET(n));
}

int get_bit(uint32_t *arr, int n) {
  uint32_t bit = arr[WORD_OFFSET(n)] & ((uint32_t)1 << BIT_OFFSET(n));
  return bit != 0;
}

void write_zeros(int bytes) {
  for (int i = 0; i < bytes; ++i)
    putc(0, fp);
}

void write_superblock() {
  fseek(fp, 0, SEEK_SET);
  if (fwrite(&super_blk, sizeof(super_blk), 1, fp) != 1) {
    fprintf(stderr, "ERROR: fwrite()\n");
    exit(EXIT_FAILURE);
  }
  // rest of the block
  write_zeros(super_blk.block_size - sizeof(super_blk));
}

void write_bitmap() {
  fseek(fp, super_blk.free_bitmap_start, SEEK_SET);
  if (fwrite(free_bitmap, BITMAP_BYTE_SIZE, 1, fp) != 1) {
    fprintf(stderr, "ERROR: fwrite()\n");
    exit(EXIT_FAILURE);
  }
  // rest of the block
  write_zeros(super_blk.block_size - BITMAP_BYTE_SIZE);
}

void write_fattable() {
  fseek(fp, super_blk.fat_table_start, SEEK_SET);
  // TODO: seekset to fattable block just in case
  if (fwrite(fat_table, sizeof(uint12), NO_BLOCKS, fp) != NO_BLOCKS) {
    fprintf(stderr, "ERROR: fwrite()\n");
    exit(EXIT_FAILURE);
  }
}
