
FLAGS := -g -Wall -W -Werror -Wno-implicit-fallthrough
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = coroutine.exe

.PHONY: all clean $(EXE)

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
