/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
extern unsigned long ctr1000;
extern int tracing;
 
/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep10(int n)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[19]++;
	}
	STATWORD ps;    
	if (n < 0  || clkruns==0)
	{
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[19] = proctab[currpid].systable.execution_time[19] + totalTime;
	}
	         return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[19] = proctab[currpid].systable.execution_time[19] + totalTime;
	}
	return(OK);
}
