KERN_DIR = /lib/modules/`uname -r`/build
EXTRA_CFLAGS := -I/home/chengl/workspace/github/fbga/driver/gpio/inc

obj-m := drv/gpio_drv.o

all:
	make -C $(KERN_DIR) M=`pwd` modules
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
