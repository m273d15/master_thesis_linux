

#include "sched.h"
#include <linux/kthread.h>

void init_plan(struct pb_rq *pb_rq)
{
	// insert the result of plan_to_c.pl below:
    pb_rq->plan[0].exec_t = 3000000000;
    pb_rq->plan[0].idle_t = 3000000000;
    pb_rq->plan[1].exec_t = 3000000000;
    pb_rq->plan[1].idle_t = 3000000000;
    pb_rq->plan[2].exec_t = 3000000000;
    pb_rq->plan[2].idle_t = 3000000000;
    pb_rq->plan[3].exec_t = 3000000000;
    pb_rq->plan[3].idle_t = 0;

}

// called by core.c sched_init
void init_pb_rq(struct pb_rq *pb_rq)
{
	pb_rq->idle_until = 0;
	pb_rq->exec_until = 0;
	pb_rq->mode = PB_DISABLED_MODE;
	pb_rq->current_entry = 0;
	pb_rq->loop_task = NULL;
	pb_rq->debug = 0;

	init_plan(pb_rq);
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

static struct task_struct *
pick_next_task_pb(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct task_struct *picked = NULL;
	u64 now;
	int current_mode, next_mode;

	now = rq->clock;

	current_mode = rq->pb.mode;
	next_mode = determine_next_mode_pd(now, rq);

	// Continue with action of mode
	if (current_mode == next_mode)
	{
		// action of EXEC is execute loop task
		if (current_mode == PB_EXEC_MODE)
			picked = rq->pb.loop_task;
		// in case of IDLE/DISABLED return NULL
	}
	// Switch of states
	else
	{
		// DISABLE --> EXEC or IDLE --> EXEC
		if ((current_mode == PB_DISABLED_MODE || current_mode == PB_IDLE_MODE) &&
			next_mode == PB_EXEC_MODE)
		{
			rq->pb.mode = next_mode;
			rq->pb.idle_until = 0;
			rq->pb.exec_until = rq->pb.plan[rq->pb.current_entry].exec_t + now;
			picked = rq->pb.loop_task;

			if (current_mode == PB_IDLE_MODE)
				printk(KERN_WARNING "IDLE,STOP,%u,%llu", rq->pb.current_entry, now);
			printk(KERN_WARNING "EXEC,START,%u,%llu", rq->pb.current_entry, now);
		}
		// EXEC --> IDLE
		else if (current_mode == PB_EXEC_MODE && next_mode == PB_IDLE_MODE)
		{
			rq->pb.mode = next_mode;
			rq->pb.idle_until = rq->pb.plan[rq->pb.current_entry].idle_t + now;
			rq->pb.exec_until = 0;

			printk(KERN_WARNING "EXEC,STOP,%u,%llu", rq->pb.current_entry, now);
			rq->pb.current_entry++;

			printk(KERN_WARNING "INDEX: %u", rq->pb.current_entry);
			if (rq->pb.current_entry < PB_PLAN_LENGTH)
			{
				printk(KERN_WARNING "IDLE,START,%u,%llu", rq->pb.current_entry, now);
			}
			else
			{
				rq->pb.mode = PB_DISABLED_MODE;
			}
		}
		// EXEC --> DISABLE
		else if (current_mode == PB_EXEC_MODE && next_mode == PB_DISABLED_MODE)
		{
			printk(KERN_WARNING "EXEC,STOP,%u,%llu", rq->pb.current_entry, now);
			printk(KERN_WARNING "END 2");
		}

		put_prev_task(rq, prev);
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
	long long now;
	int current_mode, next_mode;

	now = sched_clock();

	current_mode = rq->pb.mode;
	next_mode = determine_next_mode_pd(now, rq);


	if (current_mode == PB_EXEC_MODE && next_mode == PB_IDLE_MODE)
	{
		resched_curr(rq);
	}

	if (current_mode != PB_EXEC_MODE)
		printk(KERN_DEBUG "Tick without EXEC");
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
