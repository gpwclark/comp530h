obj-m += urrsched.o
EXTRA_CFLAGS=-std=gnu99
K=$(shell uname -r)
M=$(PWD)

all:
	make -C /lib/modules/$(K)/build M=$(M) modules
	gcc call_create.c -o call_create
	gcc is_sched_rr.c -o is_sched_rr

inst:
	make -C /lib/modules/$(K)/build M=$(M) modules_install

clean:
	make -C /lib/modules/$(K)/build M=$(M) clean
	rm call_create
	rm is_sched_rr
