
FLAGS := -g -Wall -W -Werror
LDFLAGS += -lpthread -lrt
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
EXE = producer.exe

.PHONY: all clean $(EXE)

all: $(EXE)

$(EXE): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(COBJ) $(EXE) $(RESULT)
