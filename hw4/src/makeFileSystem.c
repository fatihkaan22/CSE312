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

  printf("%d\n", block_size);
  printf("%s\n", filesystem_path);

  FILE *fp = fopen(filesystem_path, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file");
    exit(EXIT_FAILURE);
  }

  struct super_block sb;
  sb.block_size = block_size;
  sb.block_count = NO_BLOCKS; // constant: 2^12

  int disksize = sb.block_count * sb.block_size;

  for (int i = 0; i < disksize; ++i) {
    putc(0, fp);
  }

  fclose(fp);
  return 0;
}
