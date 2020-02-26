/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int wait_lock, x, max_prio = 0;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
					wait_lock = proctab[pid].wait_lckid;
					dequeue(pid);
					proctab[pid].wait_lckid = -1;
					locktab[wait_lock].lprocess[pid] = 0;
					
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

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
