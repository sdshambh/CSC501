#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int releaseall (int numlocks, int ldes1, ...)
{
	STATWORD ps;
	disable(ps);
	//kprintf("In release \n");
	unsigned long *lock;
	lock = (unsigned long *)(&ldes1);
	int lck = *(lock++),x;
	int max_priority = 0;
	int highest_prio_process;
	int syserr = 0;
	for(x = 0; x < numlocks; x++)
	{
		//kprintf("lock state %d \n",locktab[lck].lstate);
		//kprintf("Current pid %d \n",currpid);
		//kprintf("Readers %d \n",locktab[lck].lrcnt);
		if((locktab[lck].lprocess[currpid] != 1)||(locktab[lck].lstate == LFREE) || (locktab[lck].lstate == LUSED) || locktab[lck].lock_deleted[currpid] == NOTUSINGLOCK || locktab[lck].lock_deleted[currpid] == LDELETE)
		{
			syserr = 1;
		}
		//release lock
		locktab[lck].lprocess[currpid] = 0;
		proctab[currpid].process_ldesc[lck] = 0;
		locktab[lck].lock_deleted[currpid] = NOTUSINGLOCK;
		if(locktab[lck].lstate == READ)
		{
			proctab[currpid].process_lstate[lck] = LFREE;
			locktab[lck].lrcnt = locktab[lck].lrcnt - 1;
			//there is no reader--> schedule writer if any
			if(locktab[lck].lrcnt <= 0)
			{
				locktab[lck].lstate = LUSED;
				highest_prio_process = q[locktab[lck].lqtail].qprev;
				if(highest_prio_process < NPROC)
				{
					//kprintf("process in queue \n");
					//kprintf ("Queue has \n");
					//kprintf("highest prio process %d \n",highest_prio_process);
				//int proc = locktab[lck].lqhead;
				//while(q[proc].qnext!= locktab[lck].lqtail)
				//{
					//kprintf("pid in queue %d \n",proc);
					//proc = q[proc].qnext;
				//}
					locktab[lck].lstate = WRITE;
					proctab[highest_prio_process].process_lstate[lck] = WRITE;
					
					 max_priority = max_lock_wait_priority(currpid);
					if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
					{
						proctab[currpid].pprio = proctab[currpid].pprio_copy;
						proctab[currpid].pprio_copy = 0;
					}
					else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
						proctab[currpid].pprio = max_priority;
					else
					{
						if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
						{
							proctab[currpid].pprio_copy = proctab[currpid].pprio;
							proctab[currpid].pprio = max_priority;
						}
					}
					ready(getlast(locktab[lck].lqtail),RESCHNO);
				}
			}
			if (syserr)
			{
				restore(ps);
				return SYSERR;
			}
			resched();
			restore(ps);
			return(OK);
		}
		else if(locktab[lck].lstate == WRITE)
		{
			proctab[currpid].process_lstate[lck] == LFREE;
			highest_prio_process = q[locktab[lck].lqtail].qprev;
			//kprintf("highest prio process");
				if(highest_prio_process < NPROC)
				{
					//kprintf("process in queue \n");
					if(proctab[highest_prio_process].process_lstate[lck] == READ)
					{
						locktab[lck].lstate = READ;
						schedule_next_process(highest_prio_process,lck);
						
					}
					else if(proctab[highest_prio_process].process_lstate[lck] == WRITE)
					{
						schedule_write(highest_prio_process,lck);
					}
				}
				else
				{
					//if no process to schedule
					locktab[lck].lstate = LUSED;
				}
			if (syserr)
			{
			restore(ps);
			return SYSERR;
			}
			resched();
			restore(ps);
			return(OK);	
		}
	lck = *(lock++);
	}
}

//schedule read or write according to priority
void schedule_next_process(int pid,int lock)
{
	int change = 0;
	int wpid;
	int write_process_in_queue = 0;
	int diff_lrqt_time = 0;
	int previous_pid;
	int max_priority = 0;
	while(pid!=locktab[lock].lqhead)
	{
		if(write_process_in_queue <= 0)
		{
			if(proctab[pid].process_lstate[lock]==WRITE)
			{
				wpid = pid;
				write_process_in_queue = 1;
			}
			else
			{
				change=1;
				previous_pid = q[pid].qprev;
				locktab[lock].lrcnt++;
				//updating the process priorities
				 max_priority = max_lock_wait_priority(currpid);
					if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
					{
						proctab[currpid].pprio = proctab[currpid].pprio_copy;
						proctab[currpid].pprio_copy = 0;
					}
					else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
						proctab[currpid].pprio = max_priority;
					else
					{
						if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
						{
							proctab[currpid].pprio_copy = proctab[currpid].pprio;
							proctab[currpid].pprio = max_priority;
						}
					}
				ready(getlast(locktab[lock].lqtail), RESCHNO);						
			}
		}
		if(write_process_in_queue && (proctab[pid].process_lstate[lock]==READ))
		{
			//schedule the reader or writer based on 1s time limit when the priority is same
			if(proctab[pid].lpriority[lock] == proctab[wpid].lpriority[lock])
			{
				diff_lrqt_time = proctab[pid].process_lrqt_time - proctab[wpid].process_lrqt_time;
				//kprintf("difference is %d \n",diff_lrqt_time);
				if((diff_lrqt_time)<=1000)
				{
					change=1;
					previous_pid= q[pid].qprev;	
						//updating the process priorities					
					 max_priority = max_lock_wait_priority(currpid);
					if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
					{
						proctab[currpid].pprio = proctab[currpid].pprio_copy;
						proctab[currpid].pprio_copy = 0;
					}
					else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
						proctab[currpid].pprio = max_priority;
					else
					{
						if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
						{
							proctab[currpid].pprio_copy = proctab[currpid].pprio;
							proctab[currpid].pprio = max_priority;
						}
					}
					ready(dequeue(pid), RESCHNO);
				}
			}
			else if (proctab[pid].lpriority<proctab[wpid].lpriority[lock])
				break;
		}				
		if(change != 1)
		{
			pid = q[pid].qprev;
		}
		else
		{	
			change=0;
			pid = previous_pid;
		}
	}
}
//schedule write process
void schedule_write(int pid,int lock)
{
	int change = 0;
	int wpid = pid;
	int write_process_in_queue = 1;
	int diff_lrqt_time = 0;
	int previous_pid;
	int max_priority = 0;
	while(pid!=locktab[lock].lqhead)
	{
		
		if(proctab[pid].lpriority<proctab[wpid].lpriority)
		{
			break;
		}
		else if(proctab[pid].lpriority == proctab[wpid].lpriority  && proctab[pid].process_lstate[lock]==READ)
		{
			//schedule the writer according to 1s time limit
			diff_lrqt_time = proctab[pid].process_lrqt_time - proctab[wpid].process_lrqt_time;
			//kprintf("difference is %d \n",diff_lrqt_time);
			if(diff_lrqt_time<=1000)
			{
				write_process_in_queue=0;
				locktab[lock].lstate = READ;
				locktab[lock].lrcnt++;
				change=1;
				previous_pid= q[pid].qprev;
				//updating the process priorities
				max_priority = max_lock_wait_priority(currpid);
					if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
					{
						proctab[currpid].pprio = proctab[currpid].pprio_copy;
						proctab[currpid].pprio_copy = 0;
					}
					else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
						proctab[currpid].pprio = max_priority;
					else
					{
						if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
						{
							proctab[currpid].pprio_copy = proctab[currpid].pprio;
							proctab[currpid].pprio = max_priority;
						}
					}
				ready(dequeue(pid), RESCHNO);
			}
		}
		if(change)
		{
			pid = previous_pid;
			change = 0;
		}
		else
			pid = q[pid].qprev;
	}
	//if write is the highest priority process schedule write
	if(write_process_in_queue)
	{
		locktab[lock].lstate = WRITE;
					//updating the process priorities
					max_priority = max_lock_wait_priority(currpid);
					if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy > max_priority))
					{
						proctab[currpid].pprio = proctab[currpid].pprio_copy;
						proctab[currpid].pprio_copy = 0;
					}
					else if((proctab[currpid].pprio_copy > 0) && (proctab[currpid].pprio_copy <= max_priority))
						proctab[currpid].pprio = max_priority;
					else
					{
						if((proctab[currpid].pprio_copy <= 0) && max_priority > proctab[currpid].pprio)
						{
							proctab[currpid].pprio_copy = proctab[currpid].pprio;
							proctab[currpid].pprio = max_priority;
						}
					}
				ready(getlast(locktab[lock].lqtail), RESCHNO);
	}
}