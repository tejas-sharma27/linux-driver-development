#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h> // For alloc_chrdev and other functions
#include <linux/cdev.h>

//  Static Variables
static dev_t dev_num;
static struct class* cl;
// static struct device msgqueue_1;
static struct cdev msgqueue_cdev;
static char queue_data[1024];
static size_t data_occupied = 0;
static ssize_t msgqueue_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_INFO "Driver read called.\n");
    if (*f_pos > 0) {
        return 0;
    }
    if (copy_to_user(buf, queue_data, data_occupied)) {
        printk(KERN_ERR "Error in copying data from kernel to user. \n");
        return -1;
    }
    *f_pos += data_occupied;
    printk(KERN_INFO "Data copied successfully from kernel to user.\n");
    return data_occupied;
}

static ssize_t msgqueue_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver write called.\n");
    if (len > sizeof(queue_data)) {
        len = sizeof(queue_data) - 1;
        printk(KERN_WARNING "Truncating the input as data is overloaded.\n");
    }
    data_occupied = len;
    if (copy_from_user(queue_data, buf, len)) {
        printk(KERN_ERR "Error in copying data from user to kernel.\n");
        return -1;
    }
    queue_data[len] = '\0';
    printk(KERN_INFO "Data copied successfully from user to kernel.");
    return len;
}

static int msgqueue_open(struct inode* inode, struct file* filep) {
    printk(KERN_INFO "Driver open called.\n");
    return 0;
}

static int msgqueue_release(struct inode* inode, struct file* filep) {
    printk(KERN_INFO "Driver release called.\n");
    return 0;
}

struct file_operations fops = {
    .read = msgqueue_read,
    .write = msgqueue_write,
    .open = msgqueue_open,
    .release = msgqueue_release
};

static int __init msgqueue_init(void) {
    printk(KERN_INFO "Message Queue Driver loaded.");
    if ((alloc_chrdev_region(&dev_num, 0, 1, "MsgQDrvr")) < 0) {
        printk(KERN_ERR "Error in allocating chardev region.\n");
        return -1;
    }
    printk(KERN_INFO "Major Number = %d, Minor Number = %d.", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&msgqueue_cdev, &fops);
    cdev_add(&msgqueue_cdev, dev_num, 1); 
    if ((cl = class_create("msgqueue_Class")) == NULL) {
        unregister_chrdev_region(dev_num, 1);
        cdev_del(&msgqueue_cdev);
        printk(KERN_ERR "Error in create class.\n");
        return -1;
    }
    if ((device_create(cl, NULL, dev_num, NULL, "msgqueue_device")) < 0) {
        class_destroy(cl);
        unregister_chrdev_region(dev_num, 1);
        cdev_del(&msgqueue_cdev);
        printk(KERN_ERR "Error in creating device.\n");
        return -1;
    }
    printk("Device file created successfully.\n");

    return 0;
}

static void __exit msgqueue_exit(void) {
    printk(KERN_INFO "Message Queue Driver unloaded.\n");
    device_destroy(cl, dev_num);
    class_destroy(cl);
    cdev_del(&msgqueue_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk("Driver unloading successful.\n");
}

MODULE_AUTHOR("Lleo");
module_init(msgqueue_init);
module_exit(msgqueue_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A shared memory type of module");
