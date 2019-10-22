/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
extern unsigned long ctr1000;
extern int tracing;
 
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[1]++;
	}
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[1] = proctab[currpid].systable.execution_time[1] + totalTime;
	}
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[1] = proctab[currpid].systable.execution_time[1] + totalTime;
	}
	return(newprio);
}
