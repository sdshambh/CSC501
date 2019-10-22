/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
extern int tracing;
 
SYSCALL resume(int pid)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[9]++;
	}
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[9] = proctab[currpid].systable.execution_time[9] + totalTime;
	}
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[9] = proctab[currpid].systable.execution_time[9] + totalTime;
	}
	return(prio);
}
