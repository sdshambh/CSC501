/* proc.h - isbadpid */

#ifndef _PROC_H_
#define _PROC_H_

/* process table declarations and defined constants			*/

#ifndef	NPROC				/* set the number of processes	*/
#define	NPROC		30		/*  allowed if not already done	*/
#endif

#ifndef	_NFILE
#define _NFILE		20		/* # of files allowed */
#endif

#define	FDFREE		-1		/* free file descriptor */
#define PRFREE		'\002'          /* process slot is free         */


/* process state constants */

#define	PRCURR		'\001'		/* process is currently running	*/
#define	PRFREE		'\002'		/* process slot is free		*/
#define	PRREADY		'\003'		/* process is on ready queue	*/
#define	PRRECV		'\004'		/* process waiting for message	*/
#define	PRSLEEP		'\005'		/* process is sleeping		*/
#define	PRSUSP		'\006'		/* process is suspended		*/
#define	PRWAIT		'\007'		/* process is on semaphore queue*/
#define	PRTRECV		'\010'		/* process is timing a receive	*/

/* miscellaneous process definitions */

#define	PNMLEN		16		/* length of process "name"	*/

#define	NULLPROC	0		/* id of the null process; it	*/
					/*  is always eligible to run	*/
#define	BADPID		-1		/* used when invalid pid needed	*/

#define	isbadpid(x)	(x<=0 || x>=NPROC)

/* system call structure */

struct sysentry {
	
	//char syscalls[27][10];
	int execution_time[27];
	int frequency[27];
};

freemem(struct mblock *block, unsigned size);
SYSCALL chprio(int pid, int newprio);
SYSCALL getpid();
SYSCALL getprio(int pid);
SYSCALL	gettime(long *timvar);
SYSCALL kill(int pid);
SYSCALL	receive();
SYSCALL	recvclr();
SYSCALL	recvtim(int maxwait);
SYSCALL resume(int pid);
SYSCALL scount(int sem);
SYSCALL sdelete(int sem);
SYSCALL	send(int pid, WORD msg);
SYSCALL	setdev(int pid, int dev1, int dev2);
SYSCALL	setnok(int nok, int pid);
SYSCALL screate(int count);
SYSCALL signal(int sem);
SYSCALL signaln(int sem, int count);
SYSCALL	sleep(int n);
SYSCALL	sleep10(int n);
SYSCALL sleep100(int n);
SYSCALL sleep1000(int n);
SYSCALL sreset(int sem, int count);
SYSCALL stacktrace(int pid);
SYSCALL	suspend(int pid);
SYSCALL	unsleep(int pid);
SYSCALL	wait(int sem);

/* process table entry */

struct	pentry	{
	char	pstate;			/* process state: PRCURR, etc.	*/
	int	pprio;			/* process priority		*/
	int	pesp;			/* saved stack pointer		*/
	STATWORD pirmask;		/* saved interrupt mask		*/
	int	psem;			/* semaphore if process waiting	*/
	WORD	pmsg;			/* message sent to this process	*/
	char	phasmsg;		/* nonzero iff pmsg is valid	*/
	WORD	pbase;			/* base of run time stack	*/
	int	pstklen;		/* stack length			*/
	WORD	plimit;			/* lowest extent of stack	*/
	char	pname[PNMLEN];		/* process name			*/
	int	pargs;			/* initial number of arguments	*/
	WORD	paddr;			/* initial code address		*/
	WORD	pnxtkin;		/* next-of-kin notified of death*/
	Bool	ptcpumode;		/* proc is in TCP urgent mode	*/
	short	pdevs[2];		/* devices to close upon exit	*/
	int	fildes[_NFILE];		/* file - device translation	*/
	int	ppagedev;		/* pageing dgram device		*/
	int	pwaitret;
	struct sysentry systable; /* table for sys calls for each process*/
};


extern	struct	pentry proctab[];
extern	int	numproc;		/* currently active processes	*/
extern	int	nextproc;		/* search point for free slot	*/
extern	int	currpid;		/* currently executing process	*/

#endif
