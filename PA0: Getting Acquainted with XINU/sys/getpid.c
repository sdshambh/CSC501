/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
extern unsigned long ctr1000;
extern int tracing;
/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[2]++;
	}
	
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[2] = proctab[currpid].systable.execution_time[2] + totalTime;
	}
	return(currpid);
}
