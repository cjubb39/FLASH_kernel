MODULE=flash

KERN=3.9.0
#-00161-ged01b8c-dirty

obj-m += $(MODULE).o
all:
	make -C /mnt/lib/modules/$(KERN)/build M=$(PWD) modules
clean:
	make -C /mnt/lib/modules/$(KERN)/build M=$(PWD) clean
