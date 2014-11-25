obj-m += vmlogger.o

K=$(shell uname -r)
M=$(PWD)

all:
	make -C /lib/modules/$(K)/build M=$(M) modules
	gcc caller.c -o caller

inst:
	make -C /lib/modules/$(K)/build M=$(M) modules_install

clean:
	make -C /lib/modules/$(K)/build M=$(M) clean
	rm caller
