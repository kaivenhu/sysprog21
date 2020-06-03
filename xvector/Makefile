
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)
LDFLAGS += -lm

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = xvec-unittest.exe
RESULT = result.dat

.PHONY: all clean $(EXE) check

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(EXE)
	valgrind --leak-check=full ./$(EXE) check

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
