/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
extern unsigned long ctr1000;
extern int tracing;
/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	int startTime = 0, endTime = 0, totalTime = 0;
	if(tracing == 1){
	startTime = ctr1000;
	proctab[currpid].systable.frequency[13]++;
	}
	short	*nxtdev;

	if (isbadpid(pid)){
		if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[13] = proctab[currpid].systable.execution_time[13] + totalTime;
	}
		return(SYSERR);
	}
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	if(tracing == 1){
	endTime = ctr1000;
	totalTime = endTime - startTime;
	proctab[currpid].systable.execution_time[13] = proctab[currpid].systable.execution_time[13] + totalTime;
	}
	return(OK);
}
