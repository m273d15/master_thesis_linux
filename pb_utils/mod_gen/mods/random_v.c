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
                schedule();
                c++;
        }
        return 0;
}

static void init_rq(struct pb_rq *pb_rq)
{
    set_pb_plan_size(pb_rq, 5);
    set_pb_plan_entry(pb_rq, 0, 5000000000, 2000000000);
    set_pb_plan_entry(pb_rq, 1, 14000000000, 6000000000);
    set_pb_plan_entry(pb_rq, 2, 14000000000, 6000000000);
    set_pb_plan_entry(pb_rq, 3, 8000000000, 10000000000);
    set_pb_plan_entry(pb_rq, 4, 11000000000, 0);
    
}

static int __init pb_client_init(void)
{
    struct task_struct *loop_task;
    struct rq *rq;

    loop_task = kthread_create(loop_thread_func, NULL, "PB loop thread");
    loop_task->sched_class = &pb_sched_class;

    loop_task->policy = 7;
    loop_task->state = 0;

    rq = this_rq();
    init_rq(&rq->pb);

    rq->pb.loop_task = loop_task;

    return 0;
}

static void __exit pb_client_cleanup(void)
{
    struct rq *rq;
    rq = this_rq();

    // set pb_rq back to initial values
    init_pb_rq(&rq->pb);

    printk(KERN_DEBUG "Cleaning up module.\n");
}

module_init(pb_client_init);
module_exit(pb_client_cleanup);
