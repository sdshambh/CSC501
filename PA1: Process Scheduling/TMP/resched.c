/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "math.h"
#include "sched.h"
epoch = 0;
int k =0;
int usedup_quantum = 0;
int next_ready = 0;
int max_goodness = 0;
static int initial_flag = 0;
unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
int process_null();
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	
	
	
	
	/******************************Linux-like scheduler*****************************/ 
	if(getschedclass() == LINUXSCHED){
//kprintf("In LINUXSCHED \n ");

//Do this for all classes
		optr = &proctab[currpid];
		if(optr->pstate == PRCURR)
		{
		optr->pstate == PRREADY;
		insert(currpid, rdyhead, optr->pprio);
		}



	if (epoch > 0) {
			proctab[currpid].counter = preempt;
			usedup_quantum = proctab[currpid].time_quantum - proctab[currpid].counter;
			proctab[currpid].time_quantum = preempt;
			if(preempt > 0)
			proctab[currpid].goodness_val = proctab[currpid].goodness_val -  usedup_quantum;
		else
		{
			proctab[currpid].goodness_val = 0;
			proctab[currpid].time_quantum = 0;
			proctab[currpid].counter = 0;
		}
		epoch = epoch - usedup_quantum;
	}
	if(process_null())
	{
	//kprintf("Called here \n");
		epoch = 0;

	}
if(epoch == 0)
{
//for each process in ready queue set counter equal to priority only if it is the first run else no change
//for each process in ready queue do quantum = priority + floor(counter/2)
// epoch = epoch + sumofquantum of all ready processes
for(k = 1; k < NPROC; k++)
{
if(proctab[k].pstate!= PRFREE)
{
proctab[k].epoch_priority = proctab[k].pprio;
proctab[k].epoch_flag = 1;
if (initial_flag){
//kprintf("After init initializing %d \n", k);
proctab[k].time_quantum = proctab[k].pprio + proctab[k].counter/2;
if (proctab[k].time_quantum > 0)
proctab[k].goodness_val = proctab[k].pprio + proctab[k].counter;
else
proctab[k].goodness_val = 0;
}
else
{
proctab[k].time_quantum = proctab[k].pprio;
if(proctab[k].time_quantum > 0)
proctab[k].goodness_val = proctab[k].pprio;
else
proctab[k].goodness_val = 0;
initial_flag = 1;
}

epoch = epoch + proctab[k].time_quantum;

//kprintf("\n epoch value is %d, curr process is %d, %s priority %d and quantum %d, goodness %d , counter %d\n", epoch, k, proctab[k].pname, proctab[k].pprio, proctab[k].time_quantum, proctab[k].goodness_val, proctab[k].counter);
}
//kprintf("\n epoch value is %d, curr process is %d, %s priority %d and quantum %d, goodness %d , counter %d\n", epoch, k, proctab[k].pname, proctab[k].pprio, proctab[k].time_quantum, proctab[k].goodness_val, proctab[k].counter);

}
//kprintf("ALL setting on epoch 0 done \n");
}

if (epoch > 0 && !process_null()){
max_goodness = 0;
int process = q[rdytail].qprev;
while(process!=q[rdyhead].qnext)
{
if(proctab[process].goodness_val > max_goodness)
{
max_goodness = proctab[process].goodness_val;
next_ready = process;
}
process = q[process].qprev;
}
currpid = dequeue(next_ready);
nptr = &proctab[currpid];
nptr->pstate = PRCURR;
#ifdef RTCLOCK
preempt = nptr->time_quantum;
#endif
ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

return OK;
}
}
	
	
	/*******************************Exponential Distribution Scheduler********************************/
	else if(getschedclass() == EXPDISTSCHED){
		
		//int isNull = 0;
		double lambda = 0.1;
		int randomVal = (int) expdev(lambda);
		optr = &proctab[currpid];
		//kprintf("%d \n", randomVal);
		//int a = q[rdyhead].qnext;
		/*while(a!= rdytail)
		{
			kprintf(" prio=%d \n",q[a].qkey);
			a = q[a].qnext;
		}*/
		int expProcess = q[rdytail].qprev;
		int expProcess_previous = q[q[rdytail].qprev].qprev;
		// If the random value is no less than the highest priority in the ready queue, the process with the largest priority is chosen
		if (q[expProcess].qkey < randomVal)
		{
			expProcess = q[rdytail].qprev;
		}
		//If the random value is less than the lowest priority in the ready queue, the process with the lowest priority is chosen
		else if(randomVal < q[q[rdyhead].qnext].qkey)
		{
			expProcess = q[rdyhead].qnext;
		}
		else
		{
			while(expProcess!= q[rdyhead].qprev)
			{
				if((randomVal >= q[expProcess_previous].qkey)&&(randomVal <= q[expProcess].qkey))
				{
					expProcess = expProcess;
				break;	
				}
				else{
					expProcess = expProcess_previous;
					expProcess_previous = q[expProcess_previous].qprev;
				}
			}
			
			/*while((randomVal < q[expProcess_previous].qkey)&& (expProcess_previous < NPROC))
			{
				if(q[expProcess].qkey!= q[expProcess_previous].qkey)
				{
					expProcess = expProcess_previous;
				}
				expProcess_previous = q[expProcess_previous].qprev;
			}
			if (expProcess >= NPROC)
			{
				isNull = 1;
				expProcess = NULLPROC;
			}*/
		}
			
				if(optr->pstate == PRCURR)
				{
					if( (optr->pprio > randomVal)&&q[expProcess].qkey > optr->pprio)
					{
						#ifdef	RTCLOCK
					preempt = QUANTUM;
					#endif	
					return OK;
					}
				}

		if(optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
		}		
		if(expProcess < NPROC)
			{
				currpid = dequeue(expProcess);
			}
			else
			{
				currpid = EMPTY;
			}	
		
		nptr = &proctab[(currpid)];
		nptr->pstate = PRCURR;
	#ifdef	RTCLOCK
		preempt = QUANTUM;
	#endif
		//kprintf("running_2 %d \n",q[expProcess].qkey);
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	/* The OLD process returns here when resumed. */
	return OK;
		
}
		
	/******************************Default Scheduler********************************/
	/* no switch needed if current process priority higher than next*/
	else
	{
	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
	}
}


int process_null(){
	
	int queuep = q[rdytail].qprev;
	while( queuep!=q[rdyhead].qnext){
		if( (proctab[queuep].time_quantum > 0) &&(proctab[queuep].epoch_flag)){	
			return 0;
		}
		queuep = q[queuep].qprev;
	}
	//kprintf("Only Null process in queue %d\n",queuep);
	return 1;
}