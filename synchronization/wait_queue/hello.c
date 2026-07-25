#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/cdev.h>
static int counter = 0;
static struct mutex count_lock;
static dev_t dev_num;
static struct cdev my_cdev;
static struct class* hello_class;
static struct device *dev_device;
static ssize_t len;
static char data[100];
static bool data_ready = false;
static struct mutex data_lock;
wait_queue_head_t queue;
static int worker(void* data) {
    for (int i = 0; i < 1000000; i++) {
        mutex_lock(&count_lock);
        counter++;
        msleep(100);
        mutex_unlock(&count_lock);
        cond_resched();
        
    }
    return 0;
}

static ssize_t my_read(struct file *f, char __user *u, size_t t, loff_t *x)
{
	struct session *s;
	size_t bytes_to_copy;

	printk(KERN_INFO "Read is called.\n");
    wait_event_interruptible(queue, data_ready);
    printk("Wait is over. Data is ready.\n");
	if (*x >= len)
		return 0;

	bytes_to_copy = min(t, (size_t)(len - *x));

	if (copy_to_user(u, data + *x, bytes_to_copy))
		return -EFAULT;

	*x += bytes_to_copy;
	printk(KERN_INFO "Message returned to user is %s.\n", data);

	return bytes_to_copy;
}

static ssize_t my_write (struct file *f, const char __user *u, size_t t, loff_t *x) {
    printk("Write is called.\n");
    mutex_lock(&data_lock);
    if (t >= sizeof(data)) {
        t = sizeof(data);
    }
    if (copy_from_user(data, u, t)) {
        return -EFAULT;
    }
    len = t;
    data_ready = true;
    data[t] = '\0';
    mutex_unlock(&data_lock);
    wake_up_interruptible(&queue);
    return t;
}

struct file_operations fops = {
    .read = my_read,
    .write = my_write
};
static int start_module(void) 
{
    int ret;
    ret = alloc_chrdev_region(
        &dev_num,
        0,
        1,
        "hello_world"
    );
    if (ret < 0) {
        printk(KERN_ERR "alloc_chrdev_region failed.\n");
        return ret;
    }
    printk("hello world Major: %d, Minor: %d.\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "cdev_add failed.\n");
    }
    hello_class = class_create("hello_class");
    if (IS_ERR(hello_class)) {
        goto error;
    }
    dev_device = device_create(
        hello_class,
        NULL,
        dev_num,
        NULL,
        "hello_world"
    );
    if (IS_ERR(dev_device)) {
        goto error;
    }
    printk("Device file created successfully.\n");
    init_waitqueue_head(&queue);
    mutex_init(&data_lock);
    goto success;
    error:
        ret = -1;
		printk(KERN_ERR "class_create/device_create failed\n");
		cdev_del(&my_cdev);
		unregister_chrdev_region(dev_num, 1);
		return ret;
    success:
        return 0;
}

static void exit_module(void) 
{
    device_destroy(hello_class, dev_num);
    class_destroy(hello_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk("Value of counter: %d.\n", counter);
    printk("Removing the module.!\n");
}

module_init(start_module);
module_exit(exit_module);

MODULE_LICENSE("GPL");