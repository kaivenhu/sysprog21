
FLAGS := -g -Wall -W -Werror -lpthread -pthread
CFLAGS += -std=gnu11 $(FLAGS) -D_GNU_SOURCE

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = buffer.exe

.PHONY: all clean $(EXE)

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
