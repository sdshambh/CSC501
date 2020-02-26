/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
void lock_update(int pid,int prio);
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	//pptr->pprio = newprio;
	chprio_lck_sync(pid,newprio);
	restore(ps);
	return(newprio);
}


void chprio_lck_sync(int pid, int newprio)
{
	//find max wait priority 
	int max_prio = 0, x = 0;
	for(x = 0; x < NPROC; x++)
	{
		if(proctab[pid].process_ldesc[x] == 1)
		{
			if(locktab[x].wait_priority_max > max_prio)
				max_prio = locktab[x].wait_priority_max;
		}
	}
	//update process priority according to max wait priority
	if(proctab[pid].pprio_copy!=0)
	{
		if(max_prio > newprio)
		{
			proctab[pid].pprio_copy = newprio;
			proctab[pid].pprio = max_prio;
		}
		else
		{
			proctab[pid].pprio_copy = 0;
			proctab[pid].pprio = newprio;
		}
	}
	else
	{
		if(max_prio > newprio)
		{
			proctab[pid].pprio_copy = newprio;
			proctab[pid].pprio = max_prio;
		}
		else
		{
			proctab[pid].pprio_copy = 0;
			proctab[pid].pprio = newprio;
		}
	}
	
	waiting_on_lock(pid);
}



void waiting_on_lock(int pid)
{
int max_prio = 0;
		int x = 0;
//check if process is waiting on a lock
	if(proctab[pid].wait_lckid!=-1)
	{
		int wait_lock = proctab[pid].wait_lckid;
		
		//get max process priority for waiting lock
		for(x = 0; x < NPROC; x++)
		{
			if(proctab[x].wait_lckid == wait_lock)
			{
				if(proctab[x].pprio > max_prio)
				max_prio = proctab[x].pprio;
			}
		}
		//update max waiting priority 
		locktab[wait_lock].wait_priority_max = max_prio;
		
		//since max waiting priority changed update the process priorities for all process
		for(x = 0; x<NPROC; x++)
		{
			if(locktab[wait_lock].lprocess[x] == 1)
			{
				if(proctab[x].pprio_copy > 0)
				{
					if(max_prio < proctab[x].pprio_copy)
					{
					proctab[x].pprio = proctab[x].pprio_copy;
					proctab[x].pprio_copy = 0;
					}
					else
					{
					//proctab[x].pprio_copy = proctab[x].pprio;
					proctab[x].pprio = max_prio;
					//call for max check again 
					lock_update(x,max_prio);
					waiting_on_lock(x);
					}
				}
				else
				{
					if(max_prio > proctab[x].pprio)
					{
					proctab[x].pprio_copy = proctab[x].pprio;
					proctab[x].pprio = max_prio;
					//call for max check again 
					lock_update(x,max_prio);
					waiting_on_lock(x);
					}
				}
			}
		}
	}
}

void lock_update(int pid,int prio)
{
	if(proctab[pid].pprio_copy==0){		
		if(prio>proctab[pid].pprio){
			proctab[pid].pprio_copy=proctab[pid].pprio;
			proctab[pid].pprio=prio;
		}
	}
	else{
		if(prio>proctab[pid].pprio){
			proctab[pid].pprio=prio;
		}
	}	
}