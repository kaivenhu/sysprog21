
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

SRC = $(wildcard ./*.c)
OBJ = $(SRC:.c=.o)
EXE = $(SRC:.c=.exe)

.PHONY: all clean $(EXE)

all: $(EXE)

ascii.exe: CFLAGS+=-msse4

$(EXE): %.exe:%.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	rm -f $^

clean:
	rm -f $(EXE) ${OBJ}
