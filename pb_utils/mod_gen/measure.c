#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <../kernel/sched/sched.h>


MODULE_LICENSE("GPL");

static int __init pb_client_init(void)
{
    struct rq *rq;

    rq = this_rq();
    set_pb_measure_on(&rq->pb);

    return 0;
}

static void __exit pb_client_cleanup(void)
{
    struct rq *rq;
    rq = this_rq();
    set_pb_measure_off(&rq->pb);

}

module_init(pb_client_init);
module_exit(pb_client_cleanup);
