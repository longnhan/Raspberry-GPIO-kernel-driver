obj-m += aio_gpio_driver.o

PWD := $(shell pwd)
CROSS=arm-linux-gnueabihf-
# TODO: modify path to your built kernel
KER_DIR=/home/nhan/01_Projects/01_Raspberry-Pi/linux/

all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KER_DIR) M=$(PWD) modules

clean:
	make -C $(KER_DIR) M=$(PWD) clean
