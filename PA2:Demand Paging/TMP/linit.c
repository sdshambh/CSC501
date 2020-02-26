#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

struct lentry locktab[NLOCKS];
int nextlock;

void linit()
{
	int x = 0;
	while(x < NLOCKS)
	{
		locktab[x].lstate = LFREE;
		locktab[x].lrcnt = 0;
		//locktab[x].wait_priority_max = -1;
		//locktab[x].lqhead = newqueue();
		locktab[x].lqtail = 1 + (locktab[x].lqhead = newqueue());
		x++;
	}
	for(x = 0; x < NPROC; x++)
	{
		proctab[x].wait_lckid = -1;
	}
}