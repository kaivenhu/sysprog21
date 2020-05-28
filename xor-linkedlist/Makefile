
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = xor-linkedlist-unittest.exe
RESULT = result.dat

.PHONY: all clean $(EXE) check ramping

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(EXE)
	valgrind --leak-check=full ./$(EXE) check

ramping: $(EXE)
	@script/performance.sh

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
