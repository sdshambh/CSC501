/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
//kprintf("In page fault \n");
  //kprintf("To be implemented!\n");
  STATWORD ps;
  disable(ps);
  
  //read cr2 register from control_reg.c file as it holds the faulted address
  unsigned long cr2_reg_fa = read_cr2();
  int pid = currpid;
  int store = -1;
  int pageth = -1;
  int status = 0;
  int frame_avail = -1;
  int frameNumpage = -1;
  int policy = -1;
  /*Check that a is a legal address (i.e. that it has been mapped in pd). If it is not, print an error message and kill the process.*/
  status = bsm_lookup(pid,cr2_reg_fa,&store,&pageth);
  if(status == SYSERR)
  {
	  kill(currpid);
	  restore (ps);
	  return status;
  }
  unsigned long page_table = cr2_reg_fa >> 12;
  unsigned long page_directory = cr2_reg_fa >> 22;
  page_table = page_table & 0x000003ff;
  page_directory = page_directory & 0x000003ff;
  pd_t* page_directory_entry;
  page_directory_entry = proctab[pid].pdbr + page_directory * sizeof(pd_t);
  
  /*do this if there is no page directory entry present*/
  if(page_directory_entry->pd_pres == 0)
  {
	int frame_index = -1;
	int status = -1;
	int x = 0;
	status = get_frm(&frame_index);
	if(status == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	frame_avail = frame_index;
	//update frame
	frm_tab[frame_index].fr_status = FRM_MAPPED;			/* MAPPED or UNMAPPED */
    frm_tab[frame_index].fr_pid = currpid;				/* process id using this frame */
    frm_tab[frame_index].fr_vpno = -1;				/* corresponding virtual page no*/
    frm_tab[frame_index].fr_refcnt = 0;			/* reference count	*/
    frm_tab[frame_index].fr_type = FR_TBL;		/* FR_DIR, FR_TBL, FR_PAGE */
    frm_tab[frame_index].fr_dirty = 0;
	
	
	pt_t* page_table_ptr = (pt_t*)((frame_index + FRAME0)*NBPG);
	page_table_ptr->pt_base = frame_index + FRAME0;
	//initialize page table
	for(x = 0; x<NFRAMES; x++)
	{
	page_table_ptr->pt_pres	= 0;
	page_table_ptr->pt_write = 0;
	page_table_ptr->pt_user	= 0;
	page_table_ptr->pt_pwt	= 0;
	page_table_ptr->pt_pcd	= 0;
	page_table_ptr->pt_acc	= 0;
	page_table_ptr->pt_dirty = 0;
	page_table_ptr->pt_mbz	= 0;
	page_table_ptr->pt_global = 0;
	page_table_ptr->pt_avail = 0;
	page_table_ptr->pt_base	= 0;
	page_table_ptr++;
	}
	page_table_ptr = (pt_t*)((frame_index + FRAME0)*NBPG);
	//update page directory fields
	page_directory_entry->pd_pres = 1;
	page_directory_entry->pd_write = 1;
	page_directory_entry->pd_base = frame_index + FRAME0;
  }
  /*Allocating page in page table*/
  //create page table entry for indexing the page
  pt_t* page_table_entry = ((page_directory_entry->pd_base)*NBPG) + (page_table * sizeof(pt_t));
  if(page_table_entry->pt_pres == 0)
  {
	 //Obtain a free frame, f.
	status = get_frm(&frameNumpage);
	if(status == SYSERR)
	{
		restore(ps);
		return status;
	}
	//update frame
	frm_tab[frameNumpage].fr_status = FRM_MAPPED;			/* MAPPED or UNMAPPED */
    frm_tab[frameNumpage].fr_pid = currpid;				/* process id using this frame */
    frm_tab[frameNumpage].fr_vpno = cr2_reg_fa/NBPG;	/* corresponding virtual page no*/
    frm_tab[frameNumpage].fr_refcnt += 1;			/* reference count	*/
    frm_tab[frameNumpage].fr_type = FR_PAGE;		/* FR_DIR, FR_TBL, FR_PAGE */
    frm_tab[frameNumpage].fr_dirty = 0;
	
	//insert the frame of type page in SC or AGING queue according to replacement policy
	policy = grpolicy();
	if(policy == SC)
	{
		insert_frame_SC(frameNumpage);
	}
	else if(policy == AGING)
	{
		insert_frame_AGING(frameNumpage);
	}
	
	//update page table entry
	/*Update pt to mark the appropriate entry as present and set any other fields. Also set the address portion within the entry to point to frame f*/
	page_table_entry->pt_pres = 1;
	page_table_entry->pt_write = 1;
	page_table_entry->pt_user = 1;
	page_table_entry->pt_dirty = 1;
	page_table_entry->pt_base = frameNumpage + FRAME0;
	page_table_entry->pt_global = 0;
	
	//Using the backing store map, find the store s and page offset o which correspond to vp.
	bsm_lookup(pid,cr2_reg_fa,&store,&pageth);
	//Copy the page o of store s to f.
	read_bs((char *)(page_table_entry->pt_base * NBPG),store,pageth);
	//In the inverted page table, increase the reference count of the frame that holds pt. This indicates that one more of pt's entries is marked as "present."
	if(frame_avail!= -1)
	frm_tab[frame_avail].fr_refcnt +=1;
	else
	frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt +=1;
  }
  
  //restart from faulted address
  write_cr3(proctab[pid].pdbr);
  restore(ps);
  return OK;
}


