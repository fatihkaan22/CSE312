#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// TODO: header
enum command { DIR, MKDIR, RMDIR, DUMPE2FS, WRITE, READ, DEL };

struct oper {
  enum command cmd;
  char *src;
  char *dst;
};

void usage() {
  fprintf(stderr, "ERROR: Couldn't parse command line arguements\n");
  printf("Usage: ./fileSystemOper </path/to/filesystem.data> <command> "
         "<params>\n");
  exit(EXIT_FAILURE);
}

int set_command(char *str, enum command *cmd) {
  if (strcmp(str, "dir") == 0)
    *cmd = DIR;
  else if (strcmp(str, "mkdir") == 0)
    *cmd = MKDIR;
  else if (strcmp(str, "rm") == 0)
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
  return 0;
}

void print_oper(struct oper op) {
  printf("op.cmd: %d\n", op.cmd);
  printf("op.src: %s\n", op.src);
  printf("op.dst: %s\n", op.dst);
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

  printf("file  : %s\n", filesystem_path);
  print_oper(op);

  return 0;
}
