obj-m += usersync.o

K=$(shell uname -r)
M=$(PWD)

all:
	make -C /lib/modules/$(K)/build M=$(M) modules
	gcc call_create.c -o call_create
	gcc call_id.c -o call_id
	gcc call_wait.c -o call_wait
	gcc call_signal.c -o call_signal
	gcc usersync_test.c -o usersync_test

inst:
	make -C /lib/modules/$(K)/build M=$(M) modules_install

clean:
	make -C /lib/modules/$(K)/build M=$(M) clean
	rm usersync_test
	rm call_create
	rm call_id
	rm call_wait
	rm call_signal

