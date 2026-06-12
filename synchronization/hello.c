#include <linux/module.h>
#include <linux/init.h>

int start_module(void) 
{
    printk("Hello Kernel\n");
    return 0;
}

void exit_module(void) 
{
    printk("By kernel!\n");
}

module_init(start_module);
module_exit(exit_module);

MODULE_LICENSE("GPL");