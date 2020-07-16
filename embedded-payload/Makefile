
FLAGS := -g -Wall -Os
CFLAGS += -std=gnu11 $(FLAGS)

EXE = loader.exe \
	  payload.exe

.PHONY: all clean $(EXE) check

all: $(EXE)

$(EXE): %.exe:%.c
	$(CC) $(CFLAGS) -o $@ $^

check: $(EXE)
	strip -s payload.exe
	cat payload.exe >> loader.exe
	./loader.exe

clean:
	rm -f $(EXE)
