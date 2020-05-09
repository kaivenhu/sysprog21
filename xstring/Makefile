
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
UNITTEST = xs-unittest.exe

.PHONY: all check clean $(UNITTEST)

all: $(UNITTEST)

$(UNITTEST): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(UNITTEST)
	valgrind --leak-check=full ./$(UNITTEST)

clean:
	rm -f $(COBJ) $(UNITTEST)
