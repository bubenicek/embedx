
KERNEL = ~/sources/marsboard/kernel/linux-sunxi-uhab
CROSS_COMPILE=arm-linux-gnueabihf-

ccflags-y = -I$(PWD)/..

obj-m += vehabus.o
vehabus-objs := src/vehabus_drv.o 

all:
	make -C ${KERNEL} ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}  M=$(PWD) modules V=0

clean:
	make -C ${KERNEL} ARCH=mips CROSS_COMPILE=${CROSS_COMPILE} M=$(PWD) clean


