KERN_DIR = /lib/modules/`uname -r`/build
ccflags-y += -I./inc
obj-m := drv/cdma_drv.o

all:
	make -C $(KERN_DIR) M=`pwd` modules
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order