obj-m += hello_world.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BUILD_DIR := $(PWD)/build

all:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)