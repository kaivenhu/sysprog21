
MASSIF_OUT=massif-test.out

FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
UNITTEST = xs-unittest.exe

.PHONY: all check locality clean $(UNITTEST) debug

all: $(UNITTEST)

disable-cow: CFLAGS += -DDISABLE_COW
disable-cow: $(UNITTEST)

$(UNITTEST): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

check: $(UNITTEST)
	valgrind --leak-check=full ./$(UNITTEST) check

locality: $(UNITTEST)
	@script/performance.sh locality

clean:
	rm -f $(COBJ) $(UNITTEST) $(MASSIF_OUT)
