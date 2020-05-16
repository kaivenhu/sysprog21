
RESULT_FILES=massif-test.out result.png result.dat

FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = $(wildcard ./*.c)
COBJ = $(CSRC:.c=.o)
UNITTEST = xs-unittest.exe

.PHONY: all check locality benchmark clean $(UNITTEST) debug

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

benchmark: $(UNITTEST)
	@script/performance.sh benchmark

clean:
	rm -f $(COBJ) $(UNITTEST) $(RESULT_FILES)
