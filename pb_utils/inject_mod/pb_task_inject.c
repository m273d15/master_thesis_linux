#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <../kernel/sched/sched.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module that injects a loop task into the PB scheduler");

static int loop_thread_func(void *data)
{
        unsigned int c = 0;
        while (!kthread_should_stop()) {
                int a = 0;
                set_current_state(TASK_INTERRUPTIBLE);
                for (;a < 200000; a++){}
//                printk(KERN_WARNING "THREAD: looping\n");
                schedule();
                c++;
        }
        return 0;
}

static int __init hello_init(void)
{
    struct task_struct *loop_task;
    struct rq *rq;

    loop_task = kthread_create(loop_thread_func, NULL, "PB loop thread");
    loop_task->sched_class = &pb_sched_class;

    loop_task->policy = 7;
    loop_task->state = 0;

    rq = this_rq();

    rq->pb.loop_task = loop_task;

    return 0;
}

static void __exit hello_cleanup(void)
{
    printk(KERN_DEBUG "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
