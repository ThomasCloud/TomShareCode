obj-m	:= TomUsbCamDriver.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

# Prevent using an older compiler that has all kinds of dumb warnings about variable declarations. See: 
# https://stackoverflow.com/questions/2935047/how-to-use-make-and-compile-as-c99#answer-5543589 and
# https://stackoverflow.com/questions/15910064/how-to-compile-a-linux-kernel-module-using-std-gnu99#answer-15912046
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers built-in.a

