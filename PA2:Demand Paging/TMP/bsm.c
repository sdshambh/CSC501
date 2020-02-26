/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
bs_map_t bsm_tab[NBSM];
/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	/* At initialize free all the bsm and initialize it to default value*/
	STATWORD ps;
	disable(ps);
	int x = 0;
	int y = 0;
	for(x = 0 ; x < NBSM ; x++)
	{
		for(y = 0; y < NPROC ; y++)
		{
			bsm_tab[x].bs_pid[y] = 0;
			bsm_tab[x].bs_vpno[y] = -1;
			bsm_tab[x].bs_npages[y] = 0;
		}
		bsm_tab[x].bs_status = BSM_UNMAPPED;
		bsm_tab[x].bs_sem = -1;
		bsm_tab[x].bs_private_flag = 0;
		bsm_tab[x].bs_refcnt_shared = 0;
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	/*find the unmapped BSM*/
	STATWORD ps;
	disable(ps);
	*avail = -1;
	int x;
	for(x = 0 ; x < NBSM ; x++)
	{
		if(bsm_tab[x].bs_status == BSM_UNMAPPED){
			*avail = x;
			restore(ps);
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
	disable(ps);
	if((i<0) || (i >= NBSM))
	{
		restore(ps);
		return SYSERR;
	}
	int x = i;
	int y = 0;
	
		for(y = 0; y < NPROC ; y++)
		{
			bsm_tab[x].bs_pid[y] = 0;
			bsm_tab[x].bs_vpno[y] = -1;
			bsm_tab[x].bs_npages[y] = 0;
		}
		bsm_tab[x].bs_status = BSM_UNMAPPED;
		bsm_tab[x].bs_sem = -1;
		bsm_tab[x].bs_refcnt_shared = 0;
		bsm_tab[x].bs_private_flag = 0;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	/*Uniquely identify the backing store by pid and vaddr*/
	STATWORD ps;
	disable(ps);
	int x = 0;
	long vpnum = vaddr >> 12;
	for (x = 0; x < NBSM ; x++)
	{
		/*identify pid*/
		if(bsm_tab[x].bs_pid[pid] == 1)
		{
			/*identify if virtual address lies between the allocated pages for that pid (vpno <= vaddr < vpno+npages)*/
			if((bsm_tab[x].bs_vpno[pid] <= vpnum) && ((vpnum - bsm_tab[x].bs_vpno[pid]) < bsm_tab[x].bs_npages[pid]))
			{
				*store = x;
				*pageth = vpnum - bsm_tab[x].bs_vpno[pid];
				restore(ps);
				return OK;
			}
		}
	}
	//@todo: debug statement
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
	disable(ps);
	/*return error if page number greater than number of pages in backing store*/
	if((npages > NBSPG) || (npages <= 0))
	{
		restore(ps);
		return SYSERR;
	}
	/*return error if BSM id is invalid*/
	if(source < 0 || source >= NBSM)
	{
		restore(ps);
		return SYSERR;
	}
	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_pid[pid] = 1;
	bsm_tab[source].bs_vpno[pid] = vpno;
	bsm_tab[source].bs_npages[pid] = npages;
	if(bsm_tab[source].bs_private_flag)
		proctab[pid].store = source;
	bsm_tab[source].bs_refcnt_shared += 1;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	disable(ps);
	int store;
	int pageth;
	unsigned long vaddr = vpno*NBPG;
	int vpnum = -1;
	unsigned long vaddr_vpnum;
	int x = 0;
	pt_t* page_table_entry;
	pd_t* page_directory_entry;
	unsigned long page_table;
	unsigned long page_directory;
	unsigned long page;
	int frameNum = -1;
	if(isbadpid(pid))
	{
		restore(ps);
		return SYSERR;
	}
	
	if(bsm_lookup(pid, vaddr, &store , &pageth) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	vpnum = bsm_tab[store].bs_vpno[pid];
	
	for(x = 0; x < bsm_tab[store].bs_npages[pid]; x++)
	{
		vaddr_vpnum = vpnum * NBPG;
		page_directory = (vaddr_vpnum >> 22)& 0x000003ff;
		page_table = (vaddr_vpnum >> 12) & 0x000003ff;
		page_directory_entry = proctab[currpid].pdbr + (page_directory * sizeof(pd_t));
		page_table_entry = (page_directory_entry->pd_base * NBPG) + (page_table * sizeof(pt_t));
		page = page_table_entry->pt_base * NBPG;
		//if there is dirty frame(page) in page directory table->page table->page write back to corresponding backing store
		if(page_directory_entry->pd_pres == 1)
		{
			if(page_table_entry->pt_pres == 1)
			{
				frameNum = page_table_entry->pt_base - FRAME0;
				if(frm_tab[frameNum].fr_status == FRM_MAPPED)
				{
					write_bs(page,store,pageth);
					frm_tab[frameNum].fr_refcnt = frm_tab[frameNum].fr_refcnt - 1;
					if(frm_tab[frameNum].fr_refcnt == 0)
					{
						free_frm(frameNum);
						int status = -1;
						status = delete_frame_queue(frameNum);
						if(status == -1)
						{
							restore(ps);
							return SYSERR;
						}
					}
					
				}
			}
		}
		vpnum++;
	}
	bsm_tab[store].bs_refcnt_shared -= 1;
	bsm_tab[store].bs_pid[pid] = 0;
	bsm_tab[store].bs_vpno[pid] = -1;
	bsm_tab[store].bs_npages[pid] = 0;
	if(bsm_tab[store].bs_private_flag)
	{
		proctab[pid].store = -1;
		bsm_tab[store].bs_status = BSM_UNMAPPED;
	}
	else if(bsm_tab[store].bs_refcnt_shared == 0)
	{
		free_bsm(store);
	}
	restore(ps);
	return OK;
}


