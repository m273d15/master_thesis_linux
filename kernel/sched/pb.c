

#include "sched.h"
#include <linux/kthread.h>

void set_pb_measure_on(struct pb_rq *pb)
{
	pb->measure_k = PB_MEASURE_K_ON;
	pb->start = sched_clock();
	pb->ktime = 0;
	pb->kstart = 0;
}

EXPORT_SYMBOL(set_pb_measure_on);

void set_pb_measure_off(struct pb_rq *pb_rq)
{
	u64 runtime;
	u64 stop = sched_clock();

	pb_rq->measure_k = PB_MEASURE_K_OFF;
	if (stop < pb_rq->start)
	{
		printk(KERN_DEBUG "Start is greater than stop. This is a bug!\n");
	}
	runtime = stop - pb_rq->start;
	printk("Measured for %lluus detected ktime of %lluus\n", runtime, pb_rq->ktime);

	pb_rq->ktime = 0;
	pb_rq->kstart = 0;
	pb_rq->start = 0;
}

EXPORT_SYMBOL(set_pb_measure_off);

void set_pb_plan_size(struct pb_rq *pb_rq, unsigned int size)
{
	pb_rq->size = size;
}
EXPORT_SYMBOL(set_pb_plan_size);

void set_pb_plan_entry(struct pb_rq *pb_rq, unsigned int i, u64 exec_time, u64 free_time)
{
	pb_rq->plan[i].exec_time = exec_time;
	pb_rq->plan[i].free_time = free_time;
}
EXPORT_SYMBOL(set_pb_plan_entry);

// called by core.c sched_init
void init_pb_rq(struct pb_rq *pb_rq)
{
	pb_rq->free_until = 0;
	pb_rq->exec_until = 0;
	pb_rq->mode = PB_DISABLED_MODE;
	pb_rq->c_entry = 0;
	pb_rq->proxy_task = NULL;
	pb_rq->size = 0;

	pb_rq->measure_k = PB_MEASURE_K_OFF;
	pb_rq->kstart = 0;
	pb_rq->ktime = 0;
	pb_rq->start = 0;
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

static struct task_struct * pick_next_task_pb(struct rq *rq,
		struct task_struct *prev, struct rq_flags *rf)
{
	// contains task to be executed
	struct task_struct *picked = NULL;
	u64 now;
	int current_mode, next_mode;
	struct pb_rq *pb = &(rq->pb);

	now = sched_clock();

	current_mode = pb->mode;
	next_mode = determine_next_mode_pb(now, rq);

	// The mode does not change --> no behavior change is needed
	if (current_mode == next_mode)
	{
		// continue executing the task in PB_EXEC_MODE
		if (current_mode == PB_EXEC_MODE)
			picked = pb->proxy_task;
		// in case of PB_FREE_MODE/PB_DISABLED_MODE picked == NULL
	}
	// Mode change --> behavior changes
	else
	{
		// Matches:
		// switch from PB_DISABLE_MODE to PB_EXEC_MODE or
		// switch from PB_FREE_MODE to PB_EXEC_MODE
		if ((current_mode == PB_DISABLED_MODE ||
				current_mode == PB_FREE_MODE)
				&& next_mode == PB_EXEC_MODE)
		{
			pb->mode = next_mode;
			pb->free_until = 0;
			pb->exec_until = pb->plan[pb->c_entry].exec_time + now;
			picked = pb->proxy_task;

			if (current_mode == PB_FREE_MODE)
				printk(KERN_DEBUG "IDLE,STOP,%u,%llu\n", pb->c_entry, now);
			printk(KERN_DEBUG "EXEC,START,%u,%llu\n", pb->c_entry, now);
		}
		// Matches the switch from PB_EXEC_MODE to PB_FREE_MODE
		else if (current_mode == PB_EXEC_MODE &&
				next_mode == PB_FREE_MODE)
		{
			pb->mode = next_mode;
			pb->free_until = pb->plan[pb->c_entry].free_time + now;
			pb->exec_until = 0;

			printk(KERN_DEBUG "EXEC,STOP,%u,%llu\n", pb->c_entry, now);

			pb->c_entry++;
			if (pb->c_entry == pb->size)
				printk(KERN_DEBUG "PLAN DONE\n");

			// The last idle entry of a plan is ignored, since the scheduler
			// behaves after the plan execution as in the idle mode.
			if (pb->c_entry >= pb->size)
			{
				pb->mode = PB_DISABLED_MODE;
			}
			else
			{
				printk(KERN_DEBUG "IDLE,START,%u,%llu\n", pb->c_entry, now);
			}
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

	now = sched_clock();

	current_mode = rq->pb.mode;
	next_mode = determine_next_mode_pb(now, rq);

	if (current_mode == PB_EXEC_MODE && next_mode == PB_FREE_MODE)
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
