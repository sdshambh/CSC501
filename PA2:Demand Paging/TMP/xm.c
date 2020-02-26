/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  //kprintf("xmmap - to be implemented!\n");
  STATWORD ps;
  disable(ps);
  int pid = currpid;
  if((source<0) || (source >= NBSM))
  {
	restore(ps);
	return SYSERR;
  }
  if((npages > NBSPG) || (npages <= 0))
  {
	restore(ps);
	return SYSERR;
  }
  if(bsm_tab[source].bs_status == BSM_UNMAPPED)
  {
	if(bsm_map(pid, virtpage, source, npages)== SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	else
	{
		restore(ps);
		return OK;
	}
  }
  if((bsm_tab[source].bs_status == BSM_MAPPED) && (bsm_tab[source].bs_private_flag == 0) && (bsm_tab[source].bs_pid[currpid] == 0) )
  {
	 if(bsm_map(pid, virtpage, source, npages)== SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	else
	{
		restore(ps);
		return OK;
	} 
  }
  restore(ps);
  return SYSERR;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  //kprintf("To be implemented!");
  STATWORD ps;
  disable(ps);
  int status = -1;
  status = bsm_unmap(currpid,virtpage,0);
  if(status == OK)
  {
	restore(ps);
	return OK;
  }
  restore(ps);
  return SYSERR;
}
