
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = xvec-unittest.exe
RESULT = result.dat

.PHONY: all clean $(EXE)

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
