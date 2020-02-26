#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <q.h>

#define DEFAULT_LOCK_PRIO 20
#define NLOCKS 50

void reader (char *msg, int sem)
{
        //just acquire the sem/lck and do nothing(blocking writers)
		int temp = 0;
		
		for(temp = 0; temp < 5; temp++)
		{
			kprintf("%c \n",msg);
		}
		kprintf("Process %c: released sem/lck\n", msg);
		sleep(15);
}

void writer1 (char *msg, int sem)
{
	int temp = 0;
       //semaphore writer
	   wait(sem);
	    kprintf ("Process %c: acquired semaphore\n", msg);
		sleep(1);
		for(temp = 0; temp < 5; temp++)
		{
			kprintf("%c \n",msg);
		}
		signal(sem);
		kprintf ("Process %c: released semaphore\n", msg);
}

void writer2 (char *msg, int lck)
{
	int temp;
	//priority inheritance lock
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("Process %c: acquired lock\n", msg);
        sleep (1);
		for(temp = 0; temp < 5; temp++)
		{
			kprintf("%c \n",msg);
		}
        releaseall (1, lck);
		kprintf ("Process %c: released lock\n", msg);
}




void task1()
{
kprintf("--------------------------Additional Question task1------------------------- \n");
kprintf("\n");
kprintf("*****************Implementation of Semaphore*****************");
kprintf("\n");
int A,B,C; 
int s = screate(1);
if(s == SYSERR)
{
	kprintf("failed to create semaphore \n");
	return;
}
A = create(writer1, 2000, 25, "A", 2, 'A',s);
B = create(reader, 2000, 15, "B", 2, 'B',s);
C = create(writer1, 2000, 35, "C", 2, 'C',s);
resume(B);
kprintf("scheduled reader B\n");
resume(A);
kprintf("scheduled writer A\n");
resume(C);
kprintf("scheduled writer C\n");
sleep(5);
kprintf("\n");
kprintf("*****************Implementation of Reader/Writer lock*****************");
kprintf("\n");
int lck = lcreate();
if(lck == SYSERR)
{
	kprintf("failed to create lock \n");
	return;
}
int lck1 = lcreate();
A = create(writer2, 2000, 25, "A", 2, 'A',lck);
B = create(reader, 2000, 15, "B", 2, 'B',lck1);
C = create(writer2, 2000, 35, "C", 2, 'C',lck1);

resume(B);
kprintf("scheduled reader B\n");
resume(A);
kprintf("scheduled writer A\n");
resume(C);
kprintf("scheduled writer C\n");
sleep(5);


kprintf("task1 ok \n");
}


