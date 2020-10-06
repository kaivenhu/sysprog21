
FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

SRC = $(wildcard ./*.c)
EXE = $(SRC:.c=.exe)

.PHONY: all clean $(EXE)

all: $(EXE)

$(EXE): %.exe:%.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	rm -f $^

clean:
	rm -f $(EXE)
