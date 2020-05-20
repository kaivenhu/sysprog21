
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = xor-linkedlist-unittest.exe

.PHONY: all clean $(EXE) check

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(EXE)
	valgrind --leak-check=full ./$(EXE) check

clean:
	rm -f $(COBJ) $(EXE)
