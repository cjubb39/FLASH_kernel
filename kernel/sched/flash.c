#include "sched.h"
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/io.h>


struct flash_dev *flash = NULL;
EXPORT_SYMBOL(flash);


/*

enqueue_task is the class function to put the task on the list
of tasks i.e. the list of entities. The head is rq->flash_rq.queue and each 
entity has a list_head called list.

*/

static void
enqueue_task_flash(struct rq *rq, struct task_struct *p, int flags)
{
	struct flash_rq *flash_rq = &rq->flash;
	flash_rq->nr_running++;

	// TODO
	iowrite32();
}

/*

dequeue_task is called mainly from the scheduler function deactivate_task().
This function takes the task off the list of runnable tasks. This is called when
the task is done running NOT when its timeslice has expired.

*/

static void
dequeue_task_flash(struct rq *rq, struct task_struct *p, int flags)
{
	struct flash_rq *flash_rq = &rq->flash;

	if (flash_rq->nr_running)
		flash_rq->nr_running--;

	// TODO
	iowrite32();
}

/* 

This function is not really needed because flash is 
not a cooperative scheduler. However, certain programs might
want to cooperatively yield the scheduler. This is as simple as
setting the time_slice to zero and then the task will be rescheduled in
the next tick.

*/

static void
yield_task_flash(struct rq *rq)
{

	// rq->curr->flash.time_slice = 0;
	// We enqueue and then dequeue
	iowrite32();
	ioread32();
}

/*

Not needed since FLASH is not a preemptive scheduler.

*/

static void 
check_preempt_curr_flash(struct rq *rq,
		struct task_struct *p, int flags)
{
	// TODO: read the next task from FLASH
	ioread32();
}

/*

This function is called when a new task needs to be picked. Since
we are doing round-robin with FLASH, we choose the first entity
from the list of entities for that particular runqueue. 

This is also the entry point into each scheduling class. When
the core scheduler goes through the list of scheduling classes,
it checks if any class' pick_next_task function will return a task
that needs to run.

*/


static struct task_struct *pick_next_task_flash(struct rq *rq)
{
	struct task_struct *p;

	if (flash_rq->nr_running == 0)
		return NULL;

	// Get PID
	ioread32();

	// Lookup task struct from PID
	// p = container_of(entity, struct task_struct, flash);
	
	return p;
}

/*

This function is typically called by the scheduler when the current task
is going to be done. Unclear why it is called put_prev_task. This gives
us a chance to move the completed task to the end of the list. However,
we do that in task_tick_flash() depending on the available timeslice.

*/

static void put_prev_task_flash(struct rq *rq, struct task_struct *prev)
{
	// Inform the device that this task is no longer on the runqueue
	iowrite32();
}

/*

Helper function called by select_task_rq_flash()

*/

static int find_min_rq_cpu(struct task_struct *p)
{
	int cpu, min_cpu = 0, min_running = 0, first = 1;
	struct rq *rq;
	struct flash_rq *flash_rq;

	for_each_possible_cpu(cpu) {
	
		rq = cpu_rq(cpu);
		flash_rq = &rq->flash;
	
		if (first) {
			min_running = rq->flash.nr_running;
			min_cpu = cpu;
			first = 0;
			continue;
		}
	
		if (min_running > rq->flash.nr_running) {
			min_running = rq->flash.nr_running;
			min_cpu = cpu;
		}			
	}

	return min_cpu;
}

/*

When a new task has to be allotted to a rq, this function is called. Internally calls 
find_min_rq_cpu since FLASH allots a new task to the runqueue that has the least number
of tasks at a given moment

*/

static int
select_task_rq_flash(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	int min_cpu = 0;
	min_cpu = find_min_rq_cpu(p);
	return min_cpu;
}

/*

When a task sets its policy to FLASH, this function is called. At this point,
the task's policy is already set so we just have to initialize the timeslice.

*/


static void
set_curr_task_flash(struct rq *rq)
{
	// TODO: iowrite that a new task has come in
	iowrite32();
	struct task_struct *p = rq->curr;
}

/*

Every kernel tick, this function is called. We use the kernel ticks, configured by the
kernel config directive HZ to set the timeslice so it is safe to say that this is called
every kernel tick. Decrement the time_slice here and if the task has run out of its timeslice,
reschedule it. Valid values for time_slice are between 0 and FLASH_TIMESLICE. 

When a task runs out of its timeslice, it is moved to the tail of the list and then resched_curr()
is called on it. Timeslice is reset to the default value here since it needs to have a valid amount
for the next time it runs. 

*/

static void task_tick_flash(struct rq *rq, struct task_struct *curr, int queued)
{
	// TODO: Completely useless for FLASH?
	// struct sched_flash_entity *entity = &curr->flash;
	// entity->time_slice--;

	// if (entity->time_slice > 0 && entity->time_slice <= FLASH_TIMESLICE)
	// 	return;

	// entity->time_slice = FLASH_TIMESLICE;
	
	// if (rq->flash.nr_running > 1) {
		
	// 	list_move_tail(&entity->list, &rq->flash.queue);
	// 	resched_task(curr);
	// }
}

static void
prio_changed_flash(struct rq *rq, struct task_struct *p, int oldprio)
{
	// TODO: maybe never
}


static void switched_to_flash(struct rq *rq, struct task_struct *p)
{
        /*
         * If we are already running, there is nothing to be done.
         * If we are not running, we may need to preempt the current 
         * running task.
         */
        if (p->on_rq && rq->curr != p) {
	        if (!dl_task(rq->curr) && !rt_task(rq->curr)) {
                    resched_task(rq->curr);
                }
        }
}


void init_flash_rq(struct flash_rq *flash_rq, struct rq *rq)
{
	flash_rq->nr_running = 0;
	INIT_LIST_HEAD(&flash_rq->queue);
}

const struct sched_class flash_sched_class = {
	.next = &fair_sched_class,

	.enqueue_task		= enqueue_task_flash,

	.dequeue_task		= dequeue_task_flash,

	.yield_task		= yield_task_flash,

	.check_preempt_curr	= check_preempt_curr_flash,

	.pick_next_task		= pick_next_task_flash,
	.put_prev_task		= put_prev_task_flash,

	.select_task_rq		= select_task_rq_flash,

	.set_curr_task          = set_curr_task_flash,
	.task_tick		= task_tick_flash,

	.prio_changed		= prio_changed_flash,
	.switched_to		= switched_to_flash,
};
