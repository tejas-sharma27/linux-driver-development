#include <linux/init.h>
#include <linux/module.h>

static int __init driver_init(void) {
    printk(KERN_INFO "Message Queue Driver loaded.");
    return 0;
}

static void __exit driver_exit(void) {
    printk(KERN_INFO "Message Queue Driver unloaded.");
}

MODULE_AUTHOR("Lleo");
module_init(driver_init);
module_exit(driver_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A shared memory type of module");
