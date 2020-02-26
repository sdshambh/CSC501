#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
    //kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	if((bs_id<0) || (bs_id >= NBSM))
	{
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
	{
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[bs_id].bs_private_flag == 0)
	{
		free_shared_bsm(bs_id);
		if(bsm_tab[bs_id].bs_refcnt_shared == 0)
		{
			free_bsm(bs_id);
		}
	}
	else if (bsm_tab[bs_id].bs_private_flag == 1)
	{
		free_bsm(bs_id);
	}
	restore(ps);
    return OK;

}

void free_shared_bsm(int x)
{
	bsm_tab[x].bs_pid[currpid] = 0;
	bsm_tab[x].bs_vpno[currpid] = -1;
	bsm_tab[x].bs_npages[currpid] = 0;
	bsm_tab[x].bs_sem = -1;
	bsm_tab[x].bs_refcnt_shared = bsm_tab[x].bs_refcnt_shared - 1;
}
