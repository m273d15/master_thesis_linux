

#include "sched.h"
#include <linux/kthread.h>



void set_pb_plan_size(struct pb_rq *pb_rq, unsigned int size)
{
	pb_rq->size = size;
}
EXPORT_SYMBOL(set_pb_plan_size);

void set_pb_plan_entry(struct pb_rq *pb_rq, unsigned int i, u64 exec_t, u64 idle_t)
{
	pb_rq->plan[i].exec_t = exec_t;
	pb_rq->plan[i].idle_t = idle_t;
}
EXPORT_SYMBOL(set_pb_plan_entry);

// called by core.c sched_init
void init_pb_rq(struct pb_rq *pb_rq)
{
	pb_rq->idle_until = 0;
	pb_rq->exec_until = 0;
	pb_rq->mode = PB_DISABLED_MODE;
	pb_rq->current_entry = 0;
	pb_rq->loop_task = NULL;
	pb_rq->size = 0;
}
EXPORT_SYMBOL(init_pb_rq);


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
	// contains task to be executed
	struct task_struct *picked = NULL;
	u64 now;
	int current_mode, next_mode;

	now = rq->clock;

	current_mode = rq->pb.mode;
	next_mode = determine_next_mode_pd(now, rq);

	// The mode does not change --> no behavior change is needed
	if (current_mode == next_mode)
	{
		// continue executing the task in PB_EXEC_MODE
		if (current_mode == PB_EXEC_MODE)
			picked = rq->pb.loop_task;
		// in case of PB_IDLE_MODE/PB_DISABLED_MODE picked == NULL
	}
	// Mode change --> behavior changes
	else
	{
		// Matches:
		// switch from PB_DISABLE_MODE to PB_EXEC_MODE or
		// switch from PB_IDLE_MODE to PB_EXEC_MODE
		if ((current_mode == PB_DISABLED_MODE || current_mode == PB_IDLE_MODE) &&
			next_mode == PB_EXEC_MODE)
		{
			rq->pb.mode = next_mode;
			rq->pb.idle_until = 0;
			rq->pb.exec_until = rq->pb.plan[rq->pb.current_entry].exec_t + now;
			picked = rq->pb.loop_task;

			if (current_mode == PB_IDLE_MODE)
				printk(KERN_DEBUG "IDLE,STOP,%u,%llu\n", rq->pb.current_entry, now);
			printk(KERN_DEBUG "EXEC,START,%u,%llu\n", rq->pb.current_entry, now);
		}
		// Matches the switch from PB_EXEC_MODE to PB_IDLE_MODE
		else if (current_mode == PB_EXEC_MODE && next_mode == PB_IDLE_MODE)
		{
			rq->pb.mode = next_mode;
			rq->pb.idle_until = rq->pb.plan[rq->pb.current_entry].idle_t + now;
			rq->pb.exec_until = 0;

			printk(KERN_DEBUG "EXEC,STOP,%u,%llu\n", rq->pb.current_entry, now);

			rq->pb.current_entry++;
			if (rq->pb.current_entry == rq->pb.size)
				printk(KERN_DEBUG "PLAN DONE\n");

			// The last idle entry of a plan is ignored, since the scheduler
			// behaves after the plan execution as in the idle mode.
			if (rq->pb.current_entry < rq->pb.size)
			{
				printk(KERN_DEBUG "IDLE,START,%u,%llu\n", rq->pb.current_entry, now);
			}
			else
			{
				rq->pb.mode = PB_DISABLED_MODE;
			}
		}
		// TODO: I think this is dead code! The disable switch is implemented in the case above
		else if (current_mode == PB_EXEC_MODE && next_mode == PB_DISABLED_MODE)
		{
			printk(KERN_DEBUG "EXEC,STOP,%u,%llu\n", rq->pb.current_entry, now);
		}

		// Necessary to manage the preempted task
		put_prev_task(rq, prev);
	}

	return picked;
}

static void put_prev_task_pb(struct rq *rq, struct task_struct *p)
{
	// NOP
}

static void set_curr_task_pb(struct rq *rq)
{
	// NOP
}


static void task_tick_pb(struct rq *rq, struct task_struct *p, int queued)
{
	u64 now;
	int current_mode, next_mode;

	now = rq->clock;;

	current_mode = rq->pb.mode;
	next_mode = determine_next_mode_pd(now, rq);

	if (current_mode == PB_EXEC_MODE && next_mode == PB_IDLE_MODE)
	{
		resched_curr(rq);
	}
}

static unsigned int get_rr_interval_pb(struct rq *rq, struct task_struct *task)
{
	return 0;
}

static void
prio_changed_pb(struct rq *rq, struct task_struct *p, int oldprio)
{
	// NOP
}

static void switched_to_pb(struct rq *rq, struct task_struct *p)
{
	// NOP
}

static void update_curr_pb(struct rq *rq)
{
	// NOP
}

const struct sched_class pb_sched_class = {
	.next			= &dl_sched_class,
	.enqueue_task		= enqueue_task_pb, // NOP
	.dequeue_task		= dequeue_task_pb, // NOP
	.yield_task		= yield_task_pb,

	.check_preempt_curr	= check_preempt_curr_pb, // NOP

	.pick_next_task		= pick_next_task_pb,
	.put_prev_task		= put_prev_task_pb, // NOP

	.set_curr_task          = set_curr_task_pb, // NOP
	.task_tick		= task_tick_pb,

	.get_rr_interval	= get_rr_interval_pb, // NOP

	.prio_changed		= prio_changed_pb, // NOP (return 0)
	.switched_to		= switched_to_pb, // NOP

	.update_curr		= update_curr_pb, // NOP
};
EXPORT_SYMBOL(pb_sched_class);
