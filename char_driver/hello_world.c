#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#define MAGIC 'V'
#define CLEAR_BUFFER 1
#define GET_LENGTH 2
#define SET_LENGTH _IOW(MAGIC, 3, int)
static int major;
static char data[256];
static ssize_t len;
static int user_data = -1;

struct session {
	int read_count;
};
static ssize_t my_read (struct file * f, char __user *u, size_t t, loff_t * x) {
	printk("Read is called.");
	if (*x > 0) return 0;
	if (copy_to_user(u, data, len)) {
		return -EFAULT;
	}
	*x += len;
	struct session *s;
	s = f->private_data;
	s->read_count++;
	int count = s->read_count;
	printk("Read count increased to %d.", count);
	printk("Message returned to user is %s.", data);
	return len;
	
}

static ssize_t my_write (struct file *f, const char __user *u, size_t t, loff_t *x) {
	printk("Write is called");
	if (t > sizeof(data)) {
		t = sizeof(data);
	}
	if (copy_from_user(data, u, t)) {
		return -EFAULT;
	}
	len = t;
	printk("Message received from user is %s.", data);
	return t;
}

int my_open (struct inode *node, struct file *f) {
	printk("Device opened.");
	struct session *s;
	s = kmalloc(sizeof(*s), GFP_KERNEL);
	if (!s) {
		return -ENOMEM;
	}
	s->read_count = 0;
	f->private_data = s;
	return 0;
}

int my_release (struct inode *node, struct file *f) {
	printk("Device released.");
	kfree(f->private_data);
	return 0;
}

long my_ioctl (struct file* f, unsigned int cmd, unsigned long args) {
	printk("IOCTL is called.");
	switch (cmd) {
		case CLEAR_BUFFER:
			memset(data, 0, sizeof(data));
			len = 0;
			break;
		case GET_LENGTH:
			copy_to_user((int __user*)args, &len, sizeof(len));
			break;
		case SET_LENGTH:
			if (copy_from_user(&user_data, (int __user*)args, sizeof(int))) {
				return -EFAULT;
			}
			printk("Data recieved from ioctl is %d\n.", user_data);
			break;
		default:
			printk("Default");
	}
	return 0;
}
struct file_operations fileops = {
	.read = my_read,
	.write = my_write,
	.release = my_release,
	.open = my_open,
	.unlocked_ioctl = my_ioctl
};
static dev_t dev_num;
static struct cdev my_cdev;
static 	struct class *hello_class;
static int __init hello_world_init(void)
{
	
	alloc_chrdev_region(
		&dev_num, 
		0,
		1,
		"hello_world"
	);
	printk("hello_world module Major=%d, Minor=%d", MAJOR(dev_num), MINOR(dev_num));
	
	cdev_init(&my_cdev, &fileops);
	cdev_add(&my_cdev, dev_num, 1);

	hello_class = class_create("hello_class");
	device_create(
		hello_class,
		NULL,
		dev_num,
		NULL,
		"hello_world"
	);
	printk("Device file created successfully");
	return 0;
}

static void __exit hello_world_exit(void)
{
	device_destroy(hello_class, dev_num);
	class_destroy(hello_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev_num, 1);
	printk("hello_world: module unloaded\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tejas");
MODULE_DESCRIPTION("A simple hello world kernel module");
