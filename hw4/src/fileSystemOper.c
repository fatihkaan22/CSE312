#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "filesystem.h"

// TODO: header
enum command { DIR, MKDIR, RMDIR, DUMPE2FS, WRITE, READ, DEL };

struct oper {
  enum command cmd;
  char *src;
  char *dst;
};

void usage() {
  fprintf(stdout, "ERROR: Couldn't parse command line arguements\n");
  printf("Usage: ./fileSystemOper </path/to/filesystem.data> <command> "
         "<params>\n");
  exit(EXIT_FAILURE);
}

int set_command(char *str, enum command *cmd) {
  if (strcmp(str, "dir") == 0)
    *cmd = DIR;
  else if (strcmp(str, "mkdir") == 0)
    *cmd = MKDIR;
  else if (strcmp(str, "rmdir") == 0)
    *cmd = RMDIR;
  else if (strcmp(str, "dumpe2fs") == 0)
    *cmd = DUMPE2FS;
  else if (strcmp(str, "write") == 0)
    *cmd = WRITE;
  else if (strcmp(str, "read") == 0)
    *cmd = READ;
  else if (strcmp(str, "del") == 0)
    *cmd = DEL;
  else
    return -1;
  return 0;
}

bool check_params(struct oper op) {
  if (op.cmd == DUMPE2FS)
    return true;
  if (op.cmd == DIR || op.cmd == MKDIR || op.cmd == RMDIR || op.cmd == DEL)
    return (op.src != NULL);
  if (op.cmd == WRITE || op.cmd == READ)
    return (op.dst != NULL);
  return 0; // TODO: u mean return true?
}

void print_oper(struct oper op) {
  printf("op.cmd: %d\n", op.cmd);
  printf("op.src: %s\n", op.src);
  printf("op.dst: %s\n", op.dst);
}

void print_superblock() {
  puts("==== SUPERBLOCK ====");
  printf("%-22s %d\n", "Block Size:", super_blk.block_size);
  printf("%-22s %d\n", "Block Count:", super_blk.block_count);
  printf("%-22s %d\n", "Free Bitmap Start:", super_blk.free_bitmap_start);
  printf("%-22s %d\n", "Free Blocks Size:", super_blk.free_block_size);
  printf("%-22s %d\n", "Numer of Free Blocks:", super_blk.no_free_blocks);
  printf("%-22s %d\n", "FAT Table Start:", super_blk.fat_table_start);
  printf("%-22s %d\n", "FAT Table Block Size:", super_blk.fat_table_block_size);
  printf("%-22s %d\n", "Root Start:", super_blk.root_start);
  printf("%-22s %d\n", "Root Size:", super_blk.root_size);
  printf("%-22s %d\n", "Data Start:", super_blk.data_start);
  printf("%-22s %d\n", "Data Size:", super_blk.data_size);
}

void read_superblock() {
  fseek(fp, 0, SEEK_SET);
  if (fread(&super_blk, sizeof(struct super_block), 1, fp) != 1) {
    fprintf(stdout, "ERROR: fread()\n");
    exit(EXIT_FAILURE);
  }
}

void read_bitmap() {
  fseek(fp, super_blk.free_bitmap_start, SEEK_SET);
  if (fread(free_bitmap, BITMAP_BYTE_SIZE, 1, fp) != 1) {
    fprintf(stdout, "ERROR: fread()\n");
    exit(EXIT_FAILURE);
  }
}

void read_fattable() {
  fseek(fp, super_blk.fat_table_start, SEEK_SET);
  if (fread(fat_table, sizeof(uint12), NO_BLOCKS, fp) != NO_BLOCKS) {
    fprintf(stdout, "ERROR: fread()\n");
    exit(EXIT_FAILURE);
  }
}


// put into filesystem.h
uint32_t blk_addr(int block) {
  return (super_blk.root_start + (block * super_blk.block_size));
}

int first_available_block() {
  for (int i = 0; i < NO_BLOCKS; ++i) {
    if (get_bit(free_bitmap, i) == 0)
      return i;
  }
  fprintf(stdout, "ERROR: No available blocks left\n");
  return -1;
}

void pack_2b(uint32_t number, uint8_t arr[2]) { memcpy(arr, &number, 2); }

uint32_t unpack_2b(uint8_t arr[2]) {
  uint16_t number;
  memcpy(&number, arr, 2);
  return number;
}

void pack_3b(uint32_t number, uint8_t arr[3]) {
  number = number << 8;
  memcpy(arr, &number, 3);
}

uint32_t unpack_3b(uint8_t arr[3]) {
  uint32_t number;
  memcpy(&number, arr, 3);
  number = number >> 8;
  return number;
}

void set_datetime(uint8_t d[3], uint8_t t[3]) {
  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  t[0] = tm_struct->tm_hour;
  t[1] = tm_struct->tm_min;
  t[2] = tm_struct->tm_sec;
  d[0] = tm_struct->tm_mday;
  d[1] = tm_struct->tm_mon;
  d[2] = tm_struct->tm_year;
}

int read_dir_at(int addr, dir_entry *d) {
  fseek(fp, addr, SEEK_SET);
  fread(d, sizeof(dir_entry), 1, fp);
  // TODO: consider restoring fp
  return 0;
}

int write_dir_at(int addr, const dir_entry *d) {
  fseek(fp, addr, SEEK_SET);
  fwrite(d, sizeof(dir_entry), 1, fp);
  // TODO: consider restoring fp
  return 0;
}

// returns the block number of last directory in the path
int follow_path(char *tokens[128], int no_tokens, dir_entry *last) {
  bool found = true;
  int blk = 2;
  int addr = blk_addr(blk);
  for (int i = 0; i < no_tokens; ++i) {
    found = false;
    // current . entry of the directory: to obtain number of directories
    dir_entry d, parent;
    read_dir_at(addr, &parent);
    /* printf("i:%d\n", i); */
    /* printf("parent.size:%d\n", unpack_3b(parent.size)); */
    // first directory is the parent, so start with 1
    for (int j = 1; j < unpack_3b(parent.size) + 1; ++j) {
      // for every directory entry inside the current directory
      read_dir_at(addr + j * sizeof(dir_entry), &d);
      /* printf("checking: %s - %s\n", d.name, tokens[i]); */
      if (strcmp(d.name, tokens[i]) == 0) {
        /* printf("FOUND\n"); */
        blk = d.address;
        addr = blk_addr(blk);
        if (last)
          *last = d;
        found = true;
        break;
      }
    }
    if (!found) {
      fprintf(stdout, "ERROR: Path is incorrect, couldn't find: %s\n",
              tokens[i]);
      exit(EXIT_FAILURE);
    }
  }
  return blk;
}

// adds the directory inside the given parent directory
void add_dir_into(int parent_blk, dir_entry d) {
  /* printf("ADD_DIR_INTO\n"); */
  /* printf("d.name:%s\n", d.name); */
  /* printf("parent_blk:%d \n", parent_blk); */
  dir_entry parent;
  read_dir_at(blk_addr(parent_blk), &parent);
  /* printf("parent.size:%d\n", unpack_3b(parent.size)); */
  // increment no dirs of the parent
  int no_dirs = unpack_3b(parent.size);
  /* printf("no_dirs:%d\n", no_dirs); */
  pack_3b(no_dirs + 1, parent.size);
  /* printf("parent.name:%s\n", parent.name); */
  /* printf("parent.size:%d\n", unpack_3b(parent.size)); */
  write_dir_at(blk_addr(parent_blk), &parent);
  // add the directory entry
  // TODO: consdier the case if no_dirs > max size
  fseek(fp, no_dirs * sizeof(dir_entry), SEEK_CUR);
  fwrite(&d, sizeof(dir_entry), 1, fp);
}

int parse_path(char *tokens[128], int *no_tokens, char *str) {
  char delims[] = "\\\"";
  *no_tokens = 0;
  char *token = strtok(str, delims);
  while (token) {
    tokens[(*no_tokens)++] = token;
    token = strtok(NULL, delims);
  }

  /* for (int i = 0; i < no_tokens; ++i) { */
  /*   printf(">%s\n", tokens[i]); */
  /* } */

  return 0;
}

int fs_mkdir(struct oper op) {
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  char *name = tokens[no_tokens - 1];
  int parent_blk = follow_path(tokens, no_tokens - 1, NULL);
  // find free block and mark as empty
  int block = first_available_block();
  fat_table[block].x = -1;
  set_bit(free_bitmap, block);
  super_blk.no_free_blocks--;

  dir_entry d;
  strcpy(d.name, name);
  /* printf("d.name:%s\n", d.name); */
  /* printf("block:%d\n", block); */
  d.address = block;
  d.attr = ATTR_DIR;
  pack_3b(0, d.size);
  set_datetime(d.date, d.time);

  add_dir_into(parent_blk, d);
  return 0;
}

int fs_dir(struct oper op) {
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  int blk = follow_path(tokens, no_tokens, NULL);
  dir_entry parent;
  read_dir_at(blk_addr(blk), &parent);
  int no_dirs = unpack_3b(parent.size);

  dir_entry d;
  int dir_count = 0;
  int total_size = 0;
  for (int i = 0; i < no_dirs; ++i) {
    fread(&d, sizeof(dir_entry), 1, fp);
    printf("%02d-%02d-%d  ", d.date[0], d.date[1], YEAR_START + d.date[2]);
    printf("%02d:%02d  ", d.time[0], d.time[1]);
    if (d.attr == ATTR_DIR) {
      printf("<DIR>         ");
      dir_count++;
    } else {
      printf("%12d  ", unpack_3b(d.size));
      total_size += unpack_3b(d.size);
    }
    printf("%s\n", d.name);
  }
  printf("%10d File(s) %10d bytes\n", no_dirs - dir_count, total_size);
  printf("%10d Dir(s) %10d bytes free\n", dir_count,
         super_blk.no_free_blocks * super_blk.block_size);

  return 0;
}

int fs_read(struct oper op) {
  // open file to get contents
  FILE *dst_fp = fopen(op.dst, "w");
  if (dst_fp == NULL) {
    fprintf(stderr, "Cannot open file");
    exit(EXIT_FAILURE);
  }
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  dir_entry d;
  follow_path(tokens, no_tokens, &d);
  fseek(fp, blk_addr(d.address), SEEK_SET);
  for (int i = 0; i < unpack_3b(d.size); ++i) {
    char c;
    fread(&c, sizeof(char), 1, fp);
    fwrite(&c, sizeof(char), 1, dst_fp);
  }
  fclose(dst_fp);
  return 0;
}

int fs_write(struct oper op) {
  // open file to get contents
  FILE *dst_fp = fopen(op.dst, "r");
  if (dst_fp == NULL) {
    fprintf(stderr, "Cannot open file");
    exit(EXIT_FAILURE);
  }

  // TODO: consider putting int one function w/ mkdir
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  char *name = tokens[no_tokens - 1];
  int parent_blk = follow_path(tokens, no_tokens - 1, NULL);
  // find free block and mark as empty
  int block = first_available_block();
  /* printf("block:%d\n", block); */
  fat_table[block].x = -1;
  set_bit(free_bitmap, block);
  super_blk.no_free_blocks--;
  // write contents
  fseek(fp, blk_addr(block), SEEK_SET);
  char c;
  int content_size = 0;
  while ((c = fgetc(dst_fp)) != EOF) {
    /* printf("%c", c); */
    fwrite(&c, sizeof(c), 1, fp);
    content_size++;
  }
  // register directory entry
  dir_entry d;
  strcpy(d.name, name);
  d.address = block;
  d.attr = ATTR_FILE;
  pack_3b(content_size, d.size);
  set_datetime(d.date, d.time);
  add_dir_into(parent_blk, d);
  return 0;
}

int fs_del(struct oper op) {
  // TODO: all folders
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  dir_entry del, d, parent;
  int block = follow_path(tokens, no_tokens, &del);
  // mark the data block as free
  clear_bit(free_bitmap, block);
  super_blk.no_free_blocks++;
  // delete the directory entry (shift entries if necessary)
  int parent_blk = follow_path(tokens, no_tokens - 1, NULL);
  read_dir_at(blk_addr(parent_blk), &parent);
  bool shift = false;
  for (int i = 1; i < unpack_3b(parent.size) + 1; ++i) {
    read_dir_at(blk_addr(parent_blk) + i * sizeof(dir_entry), &d);
    if (strcmp(del.name, d.name) == 0) {
      shift = true;
      i++;
    }
    if (shift) {
      read_dir_at(blk_addr(parent_blk) + i * sizeof(dir_entry), &d);
      write_dir_at(blk_addr(parent_blk) + (i - 1) * sizeof(dir_entry), &d);
    }
  }
  // decrease number of directories inside the parent
  pack_3b(unpack_3b(parent.size) - 1, parent.size);
  write_dir_at(blk_addr(parent_blk), &parent);
  return 0;
}

int fs_rmdir(struct oper op) {
  char *tokens[128];
  int no_tokens;
  parse_path(tokens, &no_tokens, op.src);
  dir_entry del, d, parent;
  int block = follow_path(tokens, no_tokens, &del);
  if (del.attr != ATTR_DIR) {
    fprintf(stdout, "ERROR: Not a directory: %s\n", del.name);
    return -1;
  }
  read_dir_at(blk_addr(del.address), &d);
  if (unpack_3b(d.size) != 0) {
    fprintf(stdout, "ERROR: The directory is not empty.\n");
    return -1;
  }
  // mark the data block as free
  clear_bit(free_bitmap, block);
  super_blk.no_free_blocks++;
  // delete the directory entry (shift entries if necessary)
  int parent_blk = follow_path(tokens, no_tokens - 1, NULL);
  read_dir_at(blk_addr(parent_blk), &parent);
  bool shift = false;
  for (int i = 1; i < unpack_3b(parent.size) + 1; ++i) {
    read_dir_at(blk_addr(parent_blk) + i * sizeof(dir_entry), &d);
    if (strcmp(del.name, d.name) == 0) {
      shift = true;
      i++;
    }
    if (shift) {
      read_dir_at(blk_addr(parent_blk) + i * sizeof(dir_entry), &d);
      write_dir_at(blk_addr(parent_blk) + (i - 1) * sizeof(dir_entry), &d);
    }
  }
  // decrease number of directories inside the parent
  pack_3b(unpack_3b(parent.size) - 1, parent.size);
  write_dir_at(blk_addr(parent_blk), &parent);
  return 0;
}

void print_filesystem(int blk, int *no_dir, int *no_file) {
  int addr = blk_addr(blk);
  dir_entry parent, d;
  read_dir_at(addr, &parent);
  for (int i = 1; i < unpack_3b(parent.size) + 1; ++i) {
    fread(&d, sizeof(dir_entry), 1, fp);
    read_dir_at(addr + i * sizeof(dir_entry), &d);
    printf("%d - %s\n", d.address, d.name);
    if (d.attr == ATTR_DIR) {
      (*no_dir)++;
      print_filesystem(d.address, no_dir, no_file);
    } else {
      (*no_file)++;
    }
  }
}

int dumpe2fs() {
  print_superblock();
  print_bitmap(free_bitmap);
  // TODO: print files: hexdump way?
  int no_dir, no_file;
  no_dir = no_file = 0;
  printf("====== OCCUPIED BLOCK - FILENAME ======\n");
  print_filesystem(2, &no_dir, &no_file); // roots block
  printf("Total number of directories: %d\n", no_dir);
  printf("Total number of files: %d\n", no_file);
  return 0;
}


int main(int argc, char *argv[]) {
  char *filesystem_path;
  struct oper op;
  if (argc < 3) {
    usage();
  }
  filesystem_path = argv[1];
  if (set_command(argv[2], &op.cmd) == -1)
    usage();
  op.src = (argc > 3) ? argv[3] : NULL;
  op.dst = (argc > 4) ? argv[4] : NULL;

  if (check_params(op) == false)
    usage();

  fp = fopen(filesystem_path, "rb+");
  if (fp == NULL) {
    fprintf(stdout, "Cannot open file");
    exit(EXIT_FAILURE);
  }

  /* print_oper(op); */

  // -----------------------------------

  read_superblock();
  read_bitmap();
  read_fattable();

  switch (op.cmd) {
  case DIR:
    fs_dir(op);
    break;
  case MKDIR:
    fs_mkdir(op);
    break;
  case RMDIR:
    fs_rmdir(op);
    break;
  case DUMPE2FS:
    dumpe2fs();
    break;
  case WRITE:
    fs_write(op);
    break;
  case READ:
    fs_read(op);
    break;
  case DEL:
    fs_del(op);
    break;
  }

  write_bitmap();
  write_fattable();
  write_superblock();

  fclose(fp);

  return 0;
}
