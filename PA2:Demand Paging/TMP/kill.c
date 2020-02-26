/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
//@todo: add paging.h link in makefile 
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

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
	
	//handle process destruction
	int x = 0;
	int status = -1;
	int frameNum = -1;
	//free pages
	//free all frames and write back dirty pages to bsm
	//remove bsm mapping
	for(x = 0; x < NBSM; x++)
	{
		if(bsm_tab[x].bs_pid[pid] == 1)
		{
			status = bsm_unmap(pid, bsm_tab[x].bs_vpno[pid], 0);
			if(status == SYSERR)
			{
				restore(ps);
				return status;
			}
		}
	}
	//free page tables
	//don't free up first 4 page tables since they are shared
	for(x = 4 ; x < NFRAMES; x++)
	{
		if((frm_tab[x].fr_pid == pid ) && (frm_tab[x].fr_type == FR_TBL))
		{
			free_frm(x);
		}
	}
	//free page directory and reset pdbr
	frameNum = (proctab[pid].pdbr/NBPG) - FRAME0;
	if(frameNum > 3)
	{
		frm_tab[frameNum].fr_pid = -1;
		frm_tab[frameNum].fr_status = FRM_UNMAPPED;
		frm_tab[frameNum].fr_type = FR_PAGE;
		frm_tab[frameNum].fr_vpno = -1;
		frm_tab[frameNum].fr_refcnt = 0;
	}
	proctab[pid].pdbr = 0;
	
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

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
