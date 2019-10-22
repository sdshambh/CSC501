/* suspend.c - suspend */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
extern unsigned long ctr1000;
extern int tracing;
 
/*------------------------------------------------------------------------
 *  suspend  --  suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
SYSCALL	suspend(int pid)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[24]++;
	}
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority returned		*/

	disable(ps);
	if (isbadpid(pid) || pid==NULLPROC ||
	 ((pptr= &proctab[pid])->pstate!=PRCURR && pptr->pstate!=PRREADY)) {
		restore(ps);
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[24] = proctab[currpid].systable.execution_time[24] + totalTime;
	}
		return(SYSERR);
	}
	if (pptr->pstate == PRREADY) {
		pptr->pstate = PRSUSP;
		dequeue(pid);
	}
	else {
		pptr->pstate = PRSUSP;
		resched();
	}
	prio = pptr->pprio;
	restore(ps);
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[24] = proctab[currpid].systable.execution_time[24] + totalTime;
	}
	return(prio);
}
