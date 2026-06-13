#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
static struct task_struct* t1;
static struct task_struct* t2;
static struct task_struct* t3;
static struct task_struct* t4;
static struct task_struct* t11;
static struct task_struct* t22;
static struct task_struct* t33;
static struct task_struct* t44;
static int counter = 0;
static spinlock_t* count_lock;

static int worker(void* data) {
    for (int i = 0; i < 1000000; i++) {
        spin_lock(count_lock);
        counter++;
        spin_unlock(count_lock);
        cond_resched();
        
    }
    return 0;
}

static int start_module(void) 
{
    count_lock = kmalloc(sizeof(spinlock_t), GFP_KERNEL);
    spin_lock_init(count_lock);
    printk("Synchronization module is loaded\n");
    t1 = kthread_run(worker, NULL, "worker1");
    t2 = kthread_run(worker, NULL, "worker2");
    t3 = kthread_run(worker, NULL, "worker3");
    t4 = kthread_run(worker, NULL, "worker4");
    t11 = kthread_run(worker, NULL, "worker11");
    t22 = kthread_run(worker, NULL, "worker22");
    t33 = kthread_run(worker, NULL, "worker33");
    t44 = kthread_run(worker, NULL, "worker44");
    return 0;
}

static void exit_module(void) 
{
    printk("Value of counter: %d.\n", counter);
    printk("Removing the module.!\n");
}

module_init(start_module);
module_exit(exit_module);

MODULE_LICENSE("GPL");