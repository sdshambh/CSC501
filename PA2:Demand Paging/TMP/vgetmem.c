/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	//kprintf("To be implemented!\n");
	STATWORD ps;    
	struct	mblock	*nxt, *q, *leftover;

	disable(ps);
	if (nbytes==0 || proctab[currpid].vmemlist->mnext== (struct mblock *) NULL) {
		restore(ps);
		return( (WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);
	for (q = proctab[currpid].vmemlist,nxt=proctab[currpid].vmemlist->mnext ;
	     nxt != (struct mblock *) NULL ;
	     q=nxt,nxt=nxt->mnext)
		 {
			 /*Length of requested bytes matches return heap's last allocated address*/
		if ( q->mlen == nbytes) {
			//q->mnext = nxt->mnext;
			q->mnext = (struct mblock *)((unsigned)nxt + nbytes );
			q->mlen = 0; 
			restore(ps);
			return( (WORD *)nxt );
		}
		/*Length of requested bytes doesn't matches return heap's last allocated address with requested nbytes and decrease the length*/
		else if ( q->mlen > nbytes ) {
			//kprintf("Entered else");
			leftover = (struct mblock *)( (unsigned)nxt + nbytes );
			q->mnext = leftover;
			//leftover->mnext = nxt->mnext;
			//leftover->mlen = nxt->mlen - nbytes;
			q->mlen = q->mlen - nbytes;
			restore(ps);
			return( (WORD *)nxt );
		}
		 }
	//kprintf("ERROR 2 \n");	
	restore(ps);
	return( (WORD *)SYSERR );
	//return( SYSERR );
}


