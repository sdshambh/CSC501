#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
unsigned long ctr1000;

SYSCALL lock (int ldes1, int type, int priority) 
{
	STATWORD ps;
	disable(ps);
	int max_priority;
	//kprintf("prwait %d and LDELETE %d \n",proctab[currpid].pwaitret, LDELETE);
	if((locktab[ldes1].lstate == LFREE) || (locktab[ldes1].lock_deleted[currpid] == LDELETE) || isbadlock(ldes1) || (proctab[currpid].pwaitret == LDELETE))
	{
		restore(ps);
		return SYSERR;
	}
	proctab[currpid].process_lstate[ldes1] = type;
	proctab[currpid].lpriority[ldes1] = priority;
	
	if(locktab[ldes1].lstate == LUSED) //not yet acquired by any process
	{
		if(type == READ)
		{
			//kprintf("In LUSED read \n");
			locktab[ldes1].lstate = READ;
			locktab[ldes1].lrcnt = 	locktab[ldes1].lrcnt + 1;
			locktab[ldes1].lprocess[currpid] = 1;
			proctab[currpid].process_ldesc[ldes1] = 1;
		}
		else 
		{ 
			if(type == WRITE)
			{
				//kprintf("In LUSED write \n");
				locktab[ldes1].lstate = WRITE;
				locktab[ldes1].lprocess[currpid] = 1;
				proctab[currpid].process_ldesc[ldes1] = 1;
			}
		}
		
	}
	else //acquired by some process
	{
		int queue_head = locktab[ldes1].lqhead;
		if((locktab[ldes1].lstate == READ) && (type == READ))
		{
			//kprintf("In READ read \n");
			int highest_prio_process = q[locktab[ldes1].lqtail].qprev;
			//only current process- queue empty
			if(highest_prio_process >= NPROC || proctab[highest_prio_process].lpriority[ldes1] < priority)
			{
				//kprintf("Inside empty queue condition \n");
				proctab[currpid].process_ldesc[ldes1] = 1;
				locktab[ldes1].lprocess[currpid] = 1;
				locktab[ldes1].lrcnt = 	locktab[ldes1].lrcnt + 1;
				restore(ps);
				return(OK);
				
			}
			//the priorities are equal
			else if((highest_prio_process < NPROC ) && (proctab[highest_prio_process].lpriority[ldes1] == priority))
			{
				//kprintf("Inside same priority \n");
				proctab[currpid].process_ldesc[ldes1] = 1;
				locktab[ldes1].lprocess[currpid] = 1;
				locktab[ldes1].lrcnt = 	locktab[ldes1].lrcnt + 1;
				
				//updating priority of processes according to the max wait priority
				max_priority = max_lock_wait_priority(currpid);
				if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
				{
					proctab[currpid].pprio = proctab[currpid].pprio_copy;
					proctab[currpid].pprio_copy = 0;
				}
				else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
					proctab[currpid].pprio = max_priority;
				else
				{
					if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
					{
						proctab[currpid].pprio_copy = proctab[currpid].pprio;
						proctab[currpid].pprio = max_priority;
					}
				}
				
				restore(ps);
				return(OK);
			}
			//there is a higher priority process
			else if ( (highest_prio_process < NPROC ) && (proctab[highest_prio_process].lpriority[ldes1] > priority))
			{
				//kprintf("Inside priority inversion section \n");
				priority_inheritence_transitivity(ldes1);
				insert(currpid, queue_head, priority);
				/*kprintf ("Queue has \n");
				int proc = locktab[ldes1].lqhead;
				while(q[proc].qnext!= locktab[ldes1].lqtail)
				{
					kprintf("pid in queue %d \n",proc);
					proc = q[proc].qnext;
				}*/
				proctab[currpid].process_lrqt_time = ctr1000;
				proctab[currpid].pstate = PRWAIT;
				//proctab[currpid].pwaitret = OK;
				resched();
			}
			
		}
		else
		{
			insert(currpid, queue_head, priority);
			proctab[currpid].process_lrqt_time = ctr1000;
			proctab[currpid].pstate = PRWAIT;
			//proctab[currpid].pwaitret = OK;
			priority_inheritence_transitivity(ldes1);
			resched();
		}
	}
	//updating priority of processes according to the max wait priority
	max_priority = max_lock_wait_priority(currpid);
	if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
	{
		proctab[currpid].pprio = proctab[currpid].pprio_copy;
		proctab[currpid].pprio_copy = 0;
	}
	else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
		proctab[currpid].pprio = max_priority;
	else
	{
		if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
		{
			proctab[currpid].pprio_copy = proctab[currpid].pprio;
			proctab[currpid].pprio = max_priority;
		}
	}
if(proctab[currpid].pstate == PRWAIT)
{
	restore(ps);
    return proctab[currpid].pwaitret;
}
else
{	
	restore(ps);
	return(OK);
}
}

int max_lock_wait_priority(int pid)
{
	int max_priority = 0;
	int x = 0;
	for(x=0;x<NPROC;x++)
	{
		if(proctab[pid].process_ldesc[x] == 1)
		{
			if(max_priority < locktab[x].wait_priority_max)
			{
				max_priority = locktab[x].wait_priority_max;
			}
		}
	}
	return max_priority;
}

void priority_inheritence_transitivity(int lckdesc)
{
	int x = 0, y = 0;
	proctab[currpid].wait_lckid = lckdesc;
	if(locktab[lckdesc].wait_priority_max < proctab[currpid].pprio)
	{
		//update the max prioirty
		locktab[lckdesc].wait_priority_max = proctab[currpid].pprio;
		for(x = 0; x < NPROC; x++)
		{
			if(locktab[lckdesc].lprocess[x] == 1)
			{
				//prioirty inheritence
				if(proctab[x].pprio < locktab[lckdesc].wait_priority_max)
				{
					if(proctab[x].pprio_copy == 0) proctab[x].pprio_copy = proctab[x].pprio;
					proctab[x].pprio = locktab[lckdesc].wait_priority_max;
					//check prioroity and do the transitivity for each process
						waiting_on_lock(x);
				}
			}
		}
	}
}

