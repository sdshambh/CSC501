/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
 int delete_frame_queue(int frameNum);
 int get_frame_SC();
 void insert_frame_SC(int frameNumber);
 void insert_frame_AGING(int frameNumber);
 int get_min_in_queue();
 int get_frame_AGING();
  
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	//kprintf ("Initialize frames \n");
  //kprintf("To be implemented!\n");
  STATWORD ps;
  disable(ps);
  int x = 0;
  for(x=0; x < NFRAMES; x++)
  {
    frm_tab[x].fr_status = FRM_UNMAPPED;			/* MAPPED or UNMAPPED */
    frm_tab[x].fr_pid = -1;				/* process id using this frame */
    frm_tab[x].fr_vpno = -1;				/* corresponding virtual page no*/
    frm_tab[x].fr_refcnt = 0;			/* reference count	*/
    frm_tab[x].fr_type = FR_PAGE;		/* FR_DIR, FR_TBL, FR_PAGE */
    frm_tab[x].fr_dirty = 0;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  //kprintf("To be implemented!\n");
  STATWORD ps;
  disable(ps);
  /*This code will look for unmapped frame*/
  int x = 0;
  for(x = 0; x < NFRAMES; x++)
  {
	if (frm_tab[x].fr_status == FRM_UNMAPPED)
	  {
		  //kprintf ("get frames %d \n",x);
		*avail = x;
		restore(ps);
		return OK;
	  }
  }
  /*if all frames are mapped use replacement policy SC or AGING  to evict a frame and get free frame*/
  
  if(page_replace_policy == SC)
  { 
	int frameSC; 
	frameSC = get_frame_SC();
	*avail = frameSC;
	kprintf("Frame %d Replaced \n",frameSC + FRAME0);
	restore(ps);
	return OK;
  }
  else if(page_replace_policy == AGING)
  {
	int frameAGING;
	//kprintf("queue tail %d \n",queueTail);
	frameAGING = get_frame_AGING();
	*avail = frameAGING;
	kprintf("Frame %d Replaced \n",frameAGING + FRAME0);
	restore(ps);
	return OK;
  }
	*avail = -1;
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  //kprintf("To be implemented!\n");
  STATWORD ps;
  disable(ps);
  if(frm_tab[i].fr_status == FRM_UNMAPPED)
  {
	  restore(ps);
	  return SYSERR;
  }
  if(frm_tab[i].fr_type == FR_PAGE)
  {
	  /* handle pages */
	int actual_frame = i + FRAME0;
	int store = -1;
	int page = -1;
	int pid = frm_tab[i].fr_pid; 
	int status = -1;
	status = bsm_lookup(pid, (frm_tab[i].fr_vpno << 12), &store, &page);
	
	if(status == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	/*write back frame to BSM if the page is dirty*/
	write_bs(actual_frame*NBPG, store , page);
	int page_table = frm_tab[i].fr_vpno & 0x000003ff;
	int page_directory = (frm_tab[i].fr_vpno >> 10) & 0x000003ff;
	pd_t* page_directory_entry;
	pt_t* page_table_entry;
	page_directory_entry = proctab[pid].pdbr + (page_directory * sizeof(pd_t));
	page_table_entry = (page_directory_entry->pd_base << 12 ) + (page_table * sizeof(pt_t));
	if(page_table_entry->pt_pres == 1)
	{
		//page_table_entry->pt_pres = 0;
		/*In the inverted page table decrement the reference count of the frame occupied by pt*/
		frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt = frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt - 1;
		// print and check if frameNum and i is same --same check done
		//kprintf("i %d and frame %d ", i, frameNum);
		int frameNum = page_table_entry->pt_base - FRAME0; 
		frm_tab[frameNum].fr_status = FRM_UNMAPPED	;			/* MAPPED or UNMAPPED */
		frm_tab[frameNum].fr_pid = -1;				/* process id using this frame */
		frm_tab[frameNum].fr_vpno = -1;				/* corresponding virtual page no*/
		frm_tab[frameNum].fr_refcnt = 0;			/* reference count	*/
		frm_tab[frameNum].fr_type = FR_PAGE;		/* FR_DIR, FR_TBL, FR_PAGE */
		frm_tab[frameNum].fr_dirty = 0;
		/*Mark the appropriate entry of pt as not present.*/
		page_table_entry->pt_pres = 0;
		/*If the reference count has reached zero, you should mark the appropriate entry in pd as being not present*/
		if(frm_tab[page_directory_entry->pd_base - FRAME0].fr_refcnt == 0)
		{
			int tframeNum = page_directory_entry->pd_base - FRAME0;
			frm_tab[tframeNum].fr_status = FRM_UNMAPPED	;			/* MAPPED or UNMAPPED */
			frm_tab[tframeNum].fr_pid = -1;				/* process id using this frame */
			frm_tab[tframeNum].fr_vpno = -1;				/* corresponding virtual page no*/
			frm_tab[tframeNum].fr_refcnt = 0;			/* reference count	*/
			frm_tab[tframeNum].fr_type = FR_PAGE;		/* FR_DIR, FR_TBL, FR_PAGE */
			frm_tab[tframeNum].fr_dirty = 0;
			page_directory_entry->pd_pres = 0;
		}
	}
	//@todo: If the page being removed belongs to the current process, invalidate the TLB entry for the page vp using the invlpg instruction (see Intel Volume III/II).
	if(pid == currpid)
	{
		//kprintf ("Current pid %d\n",pid);
		//invlpg?
	}
	
	restore(ps);
	return OK;
  }
  else
  {
	  /*handle directory and page table except first 4 frames*/
		if(i>3){
			frm_tab[i].fr_status = FRM_UNMAPPED;
			frm_tab[i].fr_pid = -1;				/* process id using this frame */
			frm_tab[i].fr_vpno = -1;				/* corresponding virtual page no*/
			frm_tab[i].fr_refcnt = 0;			/* reference count	*/
			frm_tab[i].fr_type = FR_PAGE;		/* FR_DIR, FR_TBL, FR_PAGE */
			frm_tab[i].fr_dirty = 0;
		}
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frame_SC - 
 *-------------------------------------------------------------------------
 */
 int get_frame_SC()
 {
	 pd_t *page_directory_index;
	 pt_t *page_table_index;
	 int request = 0;
	 int process_id = frm_tab[queueCurr].fr_pid;
		if(queueSize == 0)
		{
			return -1;
		}
	 while (request == 0)
	 {
		//find page directory and page table offset
		int page_table = frm_tab[queueCurr].fr_vpno & 0x000003ff;
		int page_directory = (frm_tab[queueCurr].fr_vpno >> 10) & 0x000003ff;
		//find the index into the page directory
		page_directory_index = proctab[process_id].pdbr + (page_directory * sizeof(pd_t));
		//find the index into the page table
		page_table_index = (page_directory_index->pd_base << 12 ) + (page_table * sizeof(pt_t));
		if(page_table_index->pt_acc == 0)
		{
			request = 1;
			free_frm(queueCurr); 
		}
		else
		{
			page_table_index->pt_acc = 0;
			
		}
		queueCurr = SC_queue[queueCurr].nextPage;
	 }
	 return SC_queue[queueCurr].prePage;
 }
 
 /*-------------------------------------------------------------------------
 * insert_frame_SC - 
 *-------------------------------------------------------------------------
 */
 void insert_frame_SC(int frameNumber)
 {
	if (frm_tab[frameNumber].fr_type == FR_PAGE)
	{
		/*empty queue condition*/
		if((queueHead == -1) && (queueTail == -1) && (queueSize == 0))
		{
			queueHead = frameNumber;
			SC_queue[frameNumber].nextPage = frameNumber;
			SC_queue[frameNumber].prePage = frameNumber;
			queueCurr = queueHead;
			queueTail = frameNumber;
		}
		else
		{	
			SC_queue[frameNumber].prePage = queueTail;
			SC_queue[frameNumber].nextPage = SC_queue[queueTail].nextPage;
			SC_queue[SC_queue[queueTail].nextPage].prePage = frameNumber;
			SC_queue[queueTail].nextPage = frameNumber;
			queueTail = frameNumber;
		}
		queueSize++;
	}
 }
 
 /*youngest in the AGING queue*/
 int get_min_in_queue()
 {
	 int x = queueHead;
	 int returnValue = x;
	 int min = 255;
	 int temp;
	 while(x!= queueTail)
	 {	 
		 //kprintf("Value of x %d and tail is %d", x,queueTail);
		 if((AGING_queue[x].age < min) && (AGING_queue[x].age!= -1))
		 {
			 min = AGING_queue[x].age;
			 returnValue = x;
		 }
		 x = AGING_queue[x].nextPage;
	 }
	 if(AGING_queue[queueTail].age < AGING_queue[returnValue].age)
	 {
		 returnValue = queueTail;
		 min = AGING_queue[queueTail].age;
	 }
	 //if (min == 255 && AGING_queue[queueHead].age == 255)
	 //{
		// returnValue = queueHead;
		 //return returnValue;
	 //}
	 temp = queueHead;
	 while(temp!= returnValue)
	 {
		 if(AGING_queue[temp].age <= AGING_queue[returnValue].age)
		 {
			 returnValue = temp;
			 break;
		 }
		 temp = AGING_queue[temp].nextPage;
	 }
	 //kprintf ("minimum age is %d",min);
	 return returnValue;
 }
 
 
 /*-------------------------------------------------------------------------
 * get_frame_Aging - 
 *-------------------------------------------------------------------------
 */

int get_frame_AGING()
 {
	pd_t *page_directory_index;
	pt_t *page_table_index;
	int process_id; 
	queueCurr = queueHead;
	int request = 0;
	if(queueSize == 0)
	{
		return -1;
	}
	while ( (queueCurr != -1)  && (queueHead != -1))
	{
		//kprintf("Test Aging \n");
		process_id = frm_tab[queueCurr].fr_pid;
		//find page directory and page table offset
		int pt_offset = frm_tab[queueCurr].fr_vpno & 0x000003ff;
		int pd_offset = (frm_tab[queueCurr].fr_vpno >> 10) & 0x000003ff;
		//find the index into the page directory
		page_directory_index = proctab[process_id].pdbr + (pd_offset * sizeof(pd_t));
		//find the index into the page table
		page_table_index = (page_directory_index->pd_base << 12 ) + (pt_offset * sizeof(pt_t));
		AGING_queue[queueCurr].age = (AGING_queue[queueCurr].age >> 1);
		if(page_table_index->pt_acc == 1)
		{
			//kprintf("ENTERED HERE \n");
			AGING_queue[queueCurr].age = AGING_queue[queueCurr].age + 128;
			if (AGING_queue[queueCurr].age > 255)
			{
				AGING_queue[queueCurr].age = 255;
			}
		}
		queueCurr = AGING_queue[queueCurr].nextPage;
	}
	int min = get_min_in_queue();
	//kprintf( "Frame of minimun in queue %d and the head is %d", min , queueHead);
	queueCurr = min;
	
	/*while( (queueCurr != -1)  && (queueHead != -1))
	{
		if(AGING_queue[queueCurr].age <= AGING_queue[min].age)
		{
			request = 1;
			break;
		}
		queueCurr = AGING_queue[queueCurr].nextPage;
	}*/
	//if(request == 1)
	//{
		delete_frame_queue(queueCurr);
		free_frm(queueCurr);
		
	//} 
	return queueCurr;
 }

void insert_frame_AGING(int frameNumber)
 {
	 //kprintf("IN insert \n");
	if (frm_tab[frameNumber].fr_type == FR_PAGE)
	{
		/*empty queue condition*/
		if((queueHead == -1) && (queueTail == -1) && (queueSize == 0))
		{
			queueHead = frameNumber;
			AGING_queue[frameNumber].nextPage = -1;
			AGING_queue[frameNumber].prePage = -1;
			AGING_queue[frameNumber].age = 255;
			queueTail = frameNumber;
		}
		else
		{	
			AGING_queue[frameNumber].prePage = queueTail;
			AGING_queue[frameNumber].nextPage = AGING_queue[queueTail].nextPage;
			AGING_queue[frameNumber].age = 255;
			AGING_queue[queueTail].nextPage = frameNumber;
			queueTail = frameNumber;
		}
		queueSize++;
	} 
 }
 
 int delete_frame_queue(int frameNum)
 {
	 //kprintf("Queue Size %d \n",queueSize);
	 if(queueSize == 0)
	 {
		 return -1;
	 }
	 if(page_replace_policy == SC)
	 {
		 if(queueSize == 1)
		 {
			SC_queue[frameNum].nextPage = -1;
			SC_queue[frameNum].prePage = -1;
			queueSize = 0;
		    queueHead = -1;
			queueCurr = -1;
			queueTail = -1;
		 }
		 else
		 {   
			 SC_queue[SC_queue[frameNum].prePage].nextPage = SC_queue[frameNum].nextPage;
			 SC_queue[SC_queue[frameNum].nextPage].prePage = SC_queue[frameNum].prePage;
			 if(frameNum == queueTail)
			 {
				 if(queueCurr == frameNum)
				 {
					 queueCurr = queueHead;
				 }
				 queueTail = SC_queue[frameNum].prePage;
			 }
			 if(frameNum == queueHead)
			 {
				 if(queueCurr == frameNum)
				 {
					 queueCurr = SC_queue[frameNum].nextPage;
				 }
				 queueHead = SC_queue[frameNum].nextPage;
			 }
			 if((queueCurr == frameNum) && (queueCurr!= queueHead) && (queueCurr!= queueHead))
			 {
				 queueCurr = SC_queue[frameNum].nextPage;
			 }
			 SC_queue[frameNum].prePage = -1;
			 SC_queue[frameNum].nextPage = -1;
			 queueSize--;
		 }
	 }
	 else if(page_replace_policy == AGING)
	 {
		 if(queueSize == 1)
		 {
			AGING_queue[frameNum].nextPage = -1;
			AGING_queue[frameNum].prePage = -1;
			AGING_queue[frameNum].age = -1;
			queueSize = 0;
		    queueHead = -1;
			queueCurr = -1;
			queueTail = -1; 
		 }
		 else
		 {
			if(frameNum == queueHead)
			{
				AGING_queue[AGING_queue[frameNum].nextPage].prePage = -1;
				queueHead = AGING_queue[frameNum].nextPage;
				queueSize--;
				
			}
			else
			{
				if(AGING_queue[frameNum].nextPage != -1)
				{
				AGING_queue[AGING_queue[frameNum].nextPage].prePage = AGING_queue[frameNum].prePage;
				AGING_queue[AGING_queue[frameNum].prePage].nextPage = AGING_queue[frameNum].nextPage;
				}
				else
				{
					AGING_queue[AGING_queue[frameNum].prePage].nextPage = -1;
					queueTail = AGING_queue[frameNum].prePage;
				}
				queueSize--;
			}
			AGING_queue[frameNum].nextPage = -1;
			AGING_queue[frameNum].prePage = -1;
			AGING_queue[frameNum].age = -1;
		 }
	 }
	 return 0;
 }