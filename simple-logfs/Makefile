KDIR=/lib/modules/$(shell uname -r)/build
obj-m += logfs.o
ccflags-y := -std=gnu11 -Wno-declaration-after-statement \
	-Wall -W -Werror -Wno-unused-variable -Wno-unused-parameter

.PHONY: all module clean

all: module

module:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
