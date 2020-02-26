#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    //kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	if((bs_id<0) || (bs_id >= NBSM))
	{
		restore(ps);
		return SYSERR;
	}
	if((npages > NBSPG) || (npages <= 0))
	{
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
	{
		restore(ps);
		return npages;
	}
	else if((bsm_tab[bs_id].bs_status == BSM_MAPPED) && (bsm_tab[bs_id].bs_private_flag == 0))
	{
		restore(ps);
		return npages;
	}
	restore(ps);
	return SYSERR;

}
