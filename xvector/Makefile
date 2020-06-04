
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS) -DPOW_FACTOR=1.5
LDFLAGS += -lm

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = xvec-unittest.exe
RESULT = $(wildcard ./*.dat) result.png

.PHONY: all clean $(EXE) check benchmark

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(EXE)
	valgrind --leak-check=full ./$(EXE) check

benchmark: $(EXE)
	@script/performance.sh benchmark

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
