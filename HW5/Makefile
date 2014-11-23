obj-m += vmlogger.o

K=$(shell uname -r)
M=$(PWD)

all:
	make -C /lib/modules/$(K)/build M=$(M) modules
	gcc caller.c -o caller
	dd if=/dev/zero of=BigFileRandom  bs=1M  count=50
	dd if=/dev/zero of=BigFileSequential  bs=1M  count=50
	dd if=/dev/zero of=BigFileStride  bs=1M  count=50

inst:
	make -C /lib/modules/$(K)/build M=$(M) modules_install

clean:
	make -C /lib/modules/$(K)/build M=$(M) clean
	rm caller
	rm BigFile*
