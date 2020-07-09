KDIR=/lib/modules/$(shell uname -r)/build

FLAGS := -g -Wall -W -Werror
CFLAGS += -std=gnu11 $(FLAGS)

CSRC = user.c
COBJ = $(CSRC:.c=.o)
TEST = user.exe

obj-m += logfs.o
ccflags-y := -std=gnu11 -Wno-declaration-after-statement \
	-Wall -W -Werror -Wno-unused-variable -Wno-unused-parameter

.PHONY: all module check clean

all: module check

check: $(TEST)

module:
	make -C $(KDIR) M=$(PWD) modules

$(TEST): $(COBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(COBJ): %.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(COBJ) $(TEST)
