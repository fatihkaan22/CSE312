CC = gcc
CFLAGS = -pedantic-errors -Wall -std=gnu99 -g -lrt -lpthread
# CFLAGS += -DDEBUG

SRCS_MKFS := makeFileSystem.c filesystem.c
SRCS_OPER := fileSystemOper.c filesystem.c

OBJS_MKFS := $(SRCS_MKFS:.c=.o)
OBJS_OPER := $(SRCS_OPER:.c=.o)

EXEC_MKFS := makeFileSystem
EXEC_OPER := fileSystemOper

%.o: %.c $(HDRS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(EXEC_MKFS) $(EXEC_OPER)

$(EXEC_MKFS): $(OBJS_MKFS)
	$(CC) -o $@ $^ $(CFLAGS)

$(EXEC_OPER): $(OBJS_OPER)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(EXEC_MKFS) $(OBJS_MKFS)
	rm -f $(EXEC_OPER) $(OBJS_OPER)
