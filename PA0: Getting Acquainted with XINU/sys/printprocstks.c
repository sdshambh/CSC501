#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long* esp;
//static unsigned long* ebp;

void printprocstks(int priority)
{
	int k;
	unsigned long* tempsp;
	struct pentry* proc;
	//kprintf("\nCurrent pid: %d", currpid);
	kprintf("\n\nvoid printprocstks(int priority)");
	for(k=0; k<NPROC ; k++) {
	proc = &proctab[k];
	if(proctab[k].pprio <= priority) {
		continue;
	}
	if (k == currpid) {
		asm("movl %esp,esp");
		tempsp = esp;
	} else {
		tempsp = (unsigned long *)proc->pesp;
	}
	if(proctab[k].pstate!= PRFREE)
	{
	kprintf("\nProcess [%s]",proctab[k].pname);
    kprintf("\n\tpid: %d",k);
    kprintf("\n\tpriority: %d",proctab[k].pprio);
    kprintf("\n\tbase: 0x%08x",proctab[k].pbase);
	kprintf("\n\tlimit: 0x%08x",proctab[k].plimit);
    kprintf("\n\tlen: %d",proctab[k].pstklen);
    kprintf("\n\tpointer: 0x%08x",tempsp);
	}
	}
}
