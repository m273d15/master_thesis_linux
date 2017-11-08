

#include "sched.h"
#include <linux/kthread.h>

// called by core.c sched_init
void init_pb_rq(struct pb_rq *pb_rq)
{
	pb_rq->idle_until = 0;
	pb_rq->current_entry = 0;

	pb_rq->plan[0].exec_t = 3000000;
	pb_rq->plan[0].idle_t = 500000;

	pb_rq->plan[1].exec_t = 10000000;
	pb_rq->plan[1].idle_t = 100000;

	pb_rq->plan[2].exec_t = 500000;
	pb_rq->plan[2].idle_t = 10000;

	pb_rq->plan[3].exec_t = 17000000;
	// last entry does not need any idle ... obvious
	pb_rq->plan[3].idle_t = 0;
	pb_rq->loop_task = NULL;

}

// task enters the runnable state
static void
enqueue_task_pb(struct rq *rq, struct task_struct *p, int flags)
{
	// NOP
}

// task exists the runnable state
static void
dequeue_task_pb(struct rq *rq, struct task_struct *p, int flags)
{
	// NOP
}

static void yield_task_pb(struct rq *rq)
{
	// NOP
}

static void check_preempt_curr_pb(struct rq *rq, struct task_struct *p, int flags)
{
	// NOP
}

static long timeval_to_usec(struct timeval time)
{
	return time.tv_sec * 1000000 + time.tv_usec;
}

static struct task_struct *
pick_next_task_pb(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct task_struct *picked = NULL;
	struct timeval now;

	if (rq->pb.loop_task == NULL)
	{
		printk(KERN_DEBUG ">> No loop task available\n");
		printk(KERN_DEBUG "> Exits\n");
		// NO LOOP TASK life makes no sense so I do nothing
		return NULL;
	}

	do_gettimeofday(&now);


	if (rq->pb.current_entry < PB_PLAN_LENGTH)
	{
		// only the first time if nothing is set
		if (rq->pb.exec_until == 0 &&
			rq->pb.idle_until == 0)
		{
			printk(KERN_DEBUG "EXEC,START,%u,%ld\n", rq->pb.current_entry, timeval_to_usec(now));
			// set first exec time
			rq->pb.exec_until = rq->pb.plan[rq->pb.current_entry].exec_t + timeval_to_usec(now);
			picked = rq->pb.loop_task;
		}
		else
		{
			// if EXEC_MODE is set
			if (rq->pb.exec_until > 0)
			{
				// if exec_until is not elapsed, continue in EXEC_MODE
				if (rq->pb.exec_until > timeval_to_usec(now))
				{
					picked = rq->pb.loop_task;
				}
				// if exec_until is elapsed set IDLE_MODE
				else
				{
					printk(KERN_DEBUG "EXEC,STOP,%u,%ld\n", rq->pb.current_entry, timeval_to_usec(now));
					printk(KERN_DEBUG "IDLE,START,%u,%ld\n", rq->pb.current_entry, timeval_to_usec(now));
					rq->pb.exec_until = 0;
					rq->pb.idle_until = rq->pb.plan[rq->pb.current_entry].idle_t + timeval_to_usec(now);
					rq->pb.current_entry++;
				}
			}

			// if idle_until is set and elapsed set to EXEC_MODE
			if (rq->pb.idle_until > 0)
			{
				if(rq->pb.idle_until < timeval_to_usec(now))
				{
					printk(KERN_DEBUG "IDLE,STOP,%u,%ld\n", rq->pb.current_entry, timeval_to_usec(now));
					printk(KERN_DEBUG "EXEC,START,%u,%ld\n", rq->pb.current_entry, timeval_to_usec(now));
					rq->pb.idle_until = 0;
					rq->pb.exec_until = rq->pb.plan[rq->pb.current_entry].exec_t + timeval_to_usec(now);
				}
			}
		}

	}

	return picked;
}

static void put_prev_task_pb(struct rq *rq, struct task_struct *p)
{

}

static void set_curr_task_pb(struct rq *rq)
{

}

static void task_tick_pb(struct rq *rq, struct task_struct *p, int queued)
{

}

static unsigned int get_rr_interval_pb(struct rq *rq, struct task_struct *task)
{
	return 0;
}

static void
prio_changed_pb(struct rq *rq, struct task_struct *p, int oldprio)
{

}

static void switched_to_pb(struct rq *rq, struct task_struct *p)
{

}

static void update_curr_pb(struct rq *rq)
{

}

const struct sched_class pb_sched_class = {
	.next			= &dl_sched_class,
	.enqueue_task		= enqueue_task_pb,
	.dequeue_task		= dequeue_task_pb,
	.yield_task		= yield_task_pb,

	.check_preempt_curr	= check_preempt_curr_pb,

	.pick_next_task		= pick_next_task_pb,
	.put_prev_task		= put_prev_task_pb,

	.set_curr_task          = set_curr_task_pb,
	.task_tick		= task_tick_pb,

	.get_rr_interval	= get_rr_interval_pb,

	.prio_changed		= prio_changed_pb,
	.switched_to		= switched_to_pb,

	.update_curr		= update_curr_pb,
};
EXPORT_SYMBOL(pb_sched_class);
