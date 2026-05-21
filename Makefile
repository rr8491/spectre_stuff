obj-m += ptewalk.o

KDIR := /root/WSL2-Linux-Kernel
PWD := $(shell pwd)

KBUILD_EXTRA_SYMBOLS :=
KBUILD_MODPOST_WARN := 1

ccflags-y += -g0

all:
	make -C $(KDIR) M=$(PWD) SKIP_BTF=1 modules
	strip --strip-debug ptewalk.ko
	gcc -Wall -Wextra -O0 -no-pie -fno-pie accesspt_test.c -o accesspt

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f accesspt