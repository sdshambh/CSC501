#ifndef _LOCK_H_
#define _LOCK_H_

#define	LFREE	'\02'		/* this LOCK is free			*/
#define	LUSED	'\03'		/* this LOCK is used			*/
#define WRITE	'\04'		/* this LOCK is write requested	*/
#define READ	'\05'		/* this LOCK is read requested 	*/

#define LDELETE 1
#define USINGLOCK	0
#define NOTUSINGLOCK -1

#ifndef NLOCKS
#define NLOCKS 50
#endif
extern unsigned long ctr1000;
struct lentry{
					/* lock table entry		*/
	char lstate;		/* the state	*/
	int	lrcnt;		/* reader count for this lock		*/
	int	lqhead;		/* q index of head of list		*/
	int	lqtail;		/* q index of tail of list		*/
	int wait_priority_max;
	int lprocess[NPROC];
	int lock_deleted[NPROC];
	};

extern struct lentry locktab[];

extern int nextlock;
void waiting_on_lock(int pid);
void chprio_lck_sync(int pid, int newprio);
int max_lock_wait_priority(int pid);
void priority_inheritence_transitivity(int lckdesc);
void lock_update(int pid,int prio);
#define	isbadlock(l)	(l<0 || l>=NLOCKS)
#endif