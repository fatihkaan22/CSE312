#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filesystem.h"

void usage() {
  printf("Usage: ./makeFileSystem <block size> </path/to/filesystem.data>\n"
         "block size    the block size of the file system "
         "in KB (Supported values: 0.5, 1, 2, 4)\n");

  exit(EXIT_FAILURE);
}

// returns block size in bytes
int set_block_size(char *bs) {
  if (strcmp(bs, "0.5") == 0)
    return 512;
  if (strcmp(bs, "1") == 0)
    return 1024;
  if (strcmp(bs, "2") == 0)
    return 2048;
  if (strcmp(bs, "4") == 0)
    return 4096;
  return -1;
}

int main(int argc, char *argv[]) {
  char *filesystem_path;
  int block_size;
  if (argc != 3)
    usage();

  if ((block_size = set_block_size(argv[1])) == -1)
    usage();
  filesystem_path = argv[2];


  fp = fopen(filesystem_path, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file");
    exit(EXIT_FAILURE);
  }

  super_blk.block_size = block_size;
  super_blk.block_count = NO_BLOCKS;        // constant: 2^12
  super_blk.free_bitmap_start = block_size; // right afer superblock
  super_blk.free_block_size = NO_BLOCKS;
  super_blk.fat_table_start =
      super_blk.free_bitmap_start + super_blk.free_block_size;
  super_blk.fat_table_block_size =
      (FAT_TABLE_BYTE_SIZE + (super_blk.block_size) - 1) / super_blk.block_size;
  super_blk.root_start = (super_blk.fat_table_start + 16 * block_size);
  super_blk.root_size = NO_ROOT_BLOCKS * block_size;
  super_blk.data_start = super_blk.root_start + super_blk.root_size;
  super_blk.no_free_blocks = (NO_BLOCKS - NO_ROOT_BLOCKS // root directory
                              - 16                       // fat + reserved
                              - 1                        // free bitmap
                              - 1                        // superblock
  );
  super_blk.data_size = super_blk.no_free_blocks * block_size;

  // free bitmap
  /* uint32_t free_bitmap[BITMAP_WORD_SIZE]; */
  memset(free_bitmap, 0, sizeof(free_bitmap)); // init with zeros
  // mark as filled: first 2 blocks
  set_bit(free_bitmap, 0);
  set_bit(free_bitmap, 1);

  // fat table
  memset(fat_table, 0, sizeof(fat_table)); // init with zeros
  // The entries in positions 0 and 1 of the FAT are reserved
  fat_table[0].x = -1;
  fat_table[1].x = -1;
  // FAT table root dir
  int s = 2;
  for (int i = 0; i < NO_ROOT_BLOCKS - 1; ++i) {
    fat_table[s].x = s + 1;
    set_bit(free_bitmap, s);
    s++;
  }
  fat_table[s].x = -1;
  set_bit(free_bitmap, s);

  write_superblock();
  write_bitmap();
  write_fattable();

  int reserved = 1 + 1 + sizeof(fat_table) / block_size;
  // fill rest of the blocks (total - reserved) with 0
  for (int i = 0; i < super_blk.block_count - reserved; ++i) {
    write_zeros(super_blk.block_size);
  }

  fclose(fp);
  return 0;
}
