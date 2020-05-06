
CFLAGS += -std=gnu11 -g -Wall -W -Werror

SRC = $(wildcard ./*.c)
OBJ = $(SRC:.c=.o)
EXE = xs.exe

LDFLAGS += -lgtest -lgtest_main -lpthread

.PHONY: all clean $(EXE) $(OBJ)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJ) $(EXE)
