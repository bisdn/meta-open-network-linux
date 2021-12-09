#KERNELS := onl-kernel-3.16-lts-x86-64-all:amd64
#KMODULES := $(wildcard *.c)
#VENDOR := delta
#BASENAME := x86-64-delta-ag9032v1
#ARCH := x86_64
#include $(ONL)/make/kmodule.mk

obj-m := accton_as4630_54pe_poe_mcu.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
