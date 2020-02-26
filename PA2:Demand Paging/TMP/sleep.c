/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep(int n)
{
	STATWORD ps;    
	if (n<0 || clkruns==0)
		return(SYSERR);
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
		reduce_ref();
	}
	if (n > 0)
	{
		sleep10(10*n);
		reduce_ref();
	}
		return(OK);
}
/*Reducing the reference counter on every CPU cycle if the frame is not referenced*/
/*Keeping track of past references*/
void reduce_ref()
{
	if(page_replace_policy == AGING)
	{
		queueCurr = queueHead;
		int process_id;
		pd_t *page_directory_index;
		pt_t *page_table_index;
		while ( (queueCurr != -1)  && (queueHead != -1))
		{
		process_id = frm_tab[queueCurr].fr_pid;
		//find page directory and page table offset
		int pt_offset = frm_tab[queueCurr].fr_vpno & 0x000003ff;
		int pd_offset = (frm_tab[queueCurr].fr_vpno >> 10) & 0x000003ff;
		//find the index into the page directory
		page_directory_index = proctab[process_id].pdbr + (pd_offset * sizeof(pd_t));
		//find the index into the page table
		page_table_index = (page_directory_index->pd_base << 12 ) + (pt_offset * sizeof(pt_t));
		page_table_index->pt_acc -= 1;
		if(page_table_index->pt_acc < 0)
			page_table_index->pt_acc = 0;
		queueCurr = AGING_queue[queueCurr].nextPage;
		}
	}
	
}