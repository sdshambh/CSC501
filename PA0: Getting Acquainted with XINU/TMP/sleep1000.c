/* sleep1000.c - sleep1000 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
extern unsigned long ctr1000;
extern int tracing;
 
/*------------------------------------------------------------------------
 * sleep1000 --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
SYSCALL sleep1000(int n)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[21]++;
	}
	STATWORD ps;    

	if (n < 0  || clkruns==0)
	{
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[21] = proctab[currpid].systable.execution_time[21] + totalTime;
	}
	         return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep1000(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[21] = proctab[currpid].systable.execution_time[21] + totalTime;
	}
	return(OK);
}
