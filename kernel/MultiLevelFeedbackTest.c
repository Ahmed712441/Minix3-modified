#include <stdio.h>
#include <string.h>

/*
* this file contains the test for the added scheduling function
* MultiLevelFeedback
*/

// configuration.h
# define RoundRobin 1
# define QuantimSize 10
# define ShortestJobFirst 2
# define PriorityBased 3
# define MultiLevelFeedbackQueue 4
# define Default 5
# define Scheduling MultiLevelFeedbackQueue
# define User_Q_level 7
# define TopUserQueue 6
# define MiddleUserQueue 7
# define LowerUserQueue 8
# define TopUserQueueQuantum 5
# define MiddleUserQueueQuantum 10
/*
* for MultiLevelFeedbackQueue if LowerUserQueueQuantum is set to 0
* LowerUserQueue will use ShortestJobFirst
*/
# define LowerUserQueueQuantum 0
/*
* for MultiLevelFeedbackQueue set rp->p_priority to the process that you wish to enter the first queue
*/
# define FirstTime 90
// config.h
#define NR_SCHED_QUEUES 16

// proc.h
// important fields process Struct
struct proc {

	int p_priority;
	char name[30];	/* name of the process, including \0 */
	int arival_time;
	int total_time;
	int response_time;
	struct proc* p_nextready;

	char p_quantum_size;
	char p_ticks_left;
#if (Scheduling == MultiLevelFeedbackQueue)
	char process_time;
#endif


#if (Scheduling == ShortestJobFirst)
	char process_time;
#endif
#if (Scheduling == PriorityBased)
	char u_priority;
#endif

};

#define NIL_PROC          ((struct proc *) 0)

static struct proc* rdy_head[NR_SCHED_QUEUES]; /* ptrs to ready list headers */
static struct proc* rdy_tail[NR_SCHED_QUEUES]; /* ptrs to ready list tails */
static struct proc* next_ptr;

void print_user_queue() {
	struct proc* process = rdy_head[User_Q_level];

	int i = 1;
	if (process != NIL_PROC) {
		printf("process_order\t | process_name\t | process remaining time\n");
	}
	while (process != NIL_PROC) {
		printf("%d \t | %s \t | %d \n", i, process->name, process->process_time);
		i++;
		process = process->p_nextready;
	}
}
void pick_proc()
{
	/* Decide who to run now.  A new process is selected by setting 'next_ptr'.
	 * When a billable process is selected, record it in 'bill_ptr', so that the
	 * clock task can tell who to bill for system time.
	 */
	register struct proc* rp;			/* process to run */
	int q;					/* iterate over queues */

	/* Check each of the scheduling queues for ready processes. The number of
	 * queues is defined in proc.h, and priorities are set in the image table.
	 * The lowest queue contains IDLE, which is always ready.
	 */
	for (q = 0; q < NR_SCHED_QUEUES; q++) {
		if ((rp = rdy_head[q]) != NIL_PROC) {
			next_ptr = rp;			/* run process 'rp' next */
			return;
		}
	}
}
void ShortestJobFisrtSched(struct proc* rp, char user_level)
{
	rp->p_priority = user_level;
	if (rdy_head[user_level] == NIL_PROC) {
		rp->p_nextready = NIL_PROC;
		rdy_head[user_level] = rdy_tail[user_level] = rp;
		pick_proc();
		return;
	}
	struct proc* prev_process;
	struct proc* process = rdy_head[user_level];
	char placed = 0;
	while (process != NIL_PROC) {
		if (rp->process_time < process->process_time) {
			placed = 1;
			if (process == rdy_head[user_level] && (process == next_ptr)) {
				rp->p_nextready = process;
				rdy_head[user_level] = rp;
				pick_proc();
				return;
			}
			rp->p_nextready = process;
			prev_process->p_nextready = rp;
			return;
		}
		prev_process = process;
		process = process->p_nextready;
	}
	if (placed == 0) {
		rdy_tail[user_level]->p_nextready = rp;
		rdy_tail[user_level] = rp;
		rdy_tail[user_level]->p_nextready = NIL_PROC;
	}

}

void RoundRobinSched(struct proc* rp, char user_level, char quantum_size)
{

	rp->p_priority = user_level;
	char has_time = rp->p_ticks_left > 0;
	rp->p_quantum_size = quantum_size;
	if (rdy_head[user_level] == NIL_PROC) {

		rp->p_ticks_left = quantum_size;
		rp->p_nextready = NIL_PROC;
		rdy_head[user_level] = rdy_tail[user_level] = rp;
		pick_proc();
		// printf("%s entered with quantum left = %d in level = %d\n",rp->name,rp->p_ticks_left,rp->p_priority);
		return;
	}
	else if (has_time) {				/* add to head of queue */

		rp->p_nextready = rdy_head[user_level];		/* chain head of queue */
		rdy_head[user_level] = rp;
		pick_proc();                          /* set new queue head */
	}
	else {

		rp->p_ticks_left = quantum_size;
		rp->p_nextready = NIL_PROC;
		rdy_tail[user_level]->p_nextready = rp;
		rdy_tail[user_level] = rp;
	}

}
void enqueue(struct proc* rp)
{

	if ((Scheduling == Default)) { // || !isuserp(rp)){
		// default implementation
	}
	else if (Scheduling == MultiLevelFeedbackQueue) {
		char q = rp->p_priority;

		if (q == FirstTime) {

			rp->p_priority = TopUserQueue;
			RoundRobinSched(rp, TopUserQueue, TopUserQueueQuantum);
			return;
		}
		++rp->p_priority;
		if (rp->p_priority > LowerUserQueue) {
			rp->p_priority = LowerUserQueue;
		}
		q = rp->p_priority;
		if (q == MiddleUserQueue) {

			RoundRobinSched(rp, MiddleUserQueue, MiddleUserQueueQuantum);
		}
		else if (q == LowerUserQueue && LowerUserQueueQuantum > 0) {

			RoundRobinSched(rp, LowerUserQueue, LowerUserQueueQuantum);
		}
		else if (q == LowerUserQueue) {

			ShortestJobFisrtSched(rp, LowerUserQueue);
		}
	}

}


void dequeue(struct proc* rp)
/* this process is no longer runnable */
{

	int q = rp->p_priority;		/* queue to use */
	struct proc** xpp;			/* iterate over queue */
	struct proc* prev_xp;

	prev_xp = NIL_PROC;
	for (xpp = &rdy_head[q]; *xpp != NIL_PROC; xpp = &(*xpp)->p_nextready) {

		if (*xpp == rp) {				/* found process to remove */
			*xpp = (*xpp)->p_nextready;		/* replace with next chain */
			if (rp == rdy_tail[q])		/* queue tail removed */
				rdy_tail[q] = prev_xp;		/* set new tail */
			if (rp == next_ptr)/* active process removed */
				pick_proc();			/* pick new process to run */
			break;
		}
		prev_xp = *xpp;				/* save previous in chain */
	}

}

void printprocesstable() {
	printf("Table : \n\n");
	printf("process_name	 | processtime             | ArrivalTime|  Priority   \n");
	printf("excel            | 10                      | 0          | 3   \n");
	printf("word             | 20                      | 8          | 5   \n");
	printf("chrome           | 5                       | 15         | 2  \n");
	printf("teams            | 3                       | 16         | 1  \n\n");
}

void initialize(struct proc* p1, struct proc* p2, struct proc* p3, struct proc* p4) {
	p1->process_time = 10;
	p2->process_time = 20;
	p3->process_time = 5;
	p4->process_time = 3;
	p1->response_time = -1;
	p2->response_time = -1;
	p3->response_time = -1;
	p4->response_time = -1;
	p1->total_time = 10;
	p2->total_time = 20;
	p3->total_time = 5;
	p4->total_time = 3;
	p1->arival_time = 0;
	p2->arival_time = 8;
	p3->arival_time = 15;
	p4->arival_time = 16;
	p1->p_priority = FirstTime;
	p2->p_priority = FirstTime;
	p3->p_priority = FirstTime;
	p4->p_priority = FirstTime;
	p1->p_ticks_left = 0;
	p2->p_ticks_left = 0;
	p3->p_ticks_left = 0;
	p4->p_ticks_left = 0;
	strcpy(p1->name, "excel");
	strcpy(p2->name, "word");
	strcpy(p3->name, "chrome");
	strcpy(p4->name, "teams");

}

int main() {

	int turnaround_time = 0, waiting_time = 0, response_time = 0;
	struct proc p1, p2, p3, p4;
	initialize(&p1, &p2, &p3, &p4);

	printprocesstable();


	for (int i = 0; i < 10 + 20 + 5 + 3; i++) {
		if (i == 0) {
			enqueue(&p1);
		}
		else if (i == 8) {
			enqueue(&p2);
		}
		else if (i == 15) {
			enqueue(&p3);
		}
		else if (i == 16) {
			enqueue(&p4);
		}
		next_ptr->process_time--;
		next_ptr->p_ticks_left--;
		if (next_ptr != NIL_PROC) {
			printf("cycle %d : %s is now running with remaing time = %d and quantum_left=%d level=%d \n", i + 1, next_ptr->name, next_ptr->process_time, next_ptr->p_ticks_left, next_ptr->p_priority);
		}
		if (next_ptr->response_time == -1) {
			next_ptr->response_time = 0;
			response_time += i - next_ptr->arival_time;
		}
		if (next_ptr->process_time == 0) {
			printf("%s finished \n", next_ptr->name);
			turnaround_time += (i - next_ptr->arival_time);
			waiting_time += (i - next_ptr->arival_time - next_ptr->total_time + 1);

			dequeue(next_ptr);
		}
		if (next_ptr->p_ticks_left <= 0) {
			struct proc* process = next_ptr;

			dequeue(process);
			enqueue(process);
		}

	}

	printf("Total waiting time : %d \nTotal turnaround time : %d \nTotal response time: %d \n", waiting_time, turnaround_time, response_time);
	return 0;
}