#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

SYSCALL lcreate (void)
{
	STATWORD ps;    
	int	lock;

	disable(ps);
	if ((lock=newlock())==SYSERR) {
		restore(ps);
		return(SYSERR);
	}
	//semaph[sem].semcnt = count;
	/* sqhead and sqtail were initialized at system startup */
	restore(ps);
	return(lock);
}


LOCAL int newlock()
{
	int	lock;
	int	i = 0;

	while (i < NLOCKS ){
		lock = nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locktab[lock].lstate == LFREE) {
			locktab[lock].lstate = LUSED;
			locktab[lock].lrcnt = 0;
			return(lock);
		}
		i++;
	}
	return(SYSERR);
}
