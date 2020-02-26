#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

SYSCALL ldelete(int lock)
{
	STATWORD ps;    
	int	pid, x;
	struct	lentry	*lckptr;

	disable(ps);
	if (isbadlock(lock) || locktab[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lckptr = &locktab[lock];
	lckptr->lstate = LFREE;
	for(x = 0; x< NPROC; x++)
	{
		if(locktab[lock].lock_deleted[x] == USINGLOCK || locktab[lock].lock_deleted[x] == NOTUSINGLOCK)
			locktab[lock].lock_deleted[x] = LDELETE;
		if(proctab[x].process_ldesc[lock])
		proctab[x].pwaitret = LDELETE;
	}
	if (nonempty(lckptr->lqhead)) {
		while( (pid=getfirst(lckptr->lqhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = LDELETE;
		    ready(pid,RESCHNO);
		  }
		  //kprintf("process delete lock \n");
		resched();
		restore(ps);
		return (DELETED); //return DELETED when lock is deleted
	}
	//kprintf("Successful delete \n");
	restore(ps);
	return(OK);
}