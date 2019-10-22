/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
//#include "lab0.h"
int prX;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
prch(c)
char c;
{
	int i;
	sleep(5);	
}
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	kprintf("zfunction value outputs to 0x%lx \n",zfunction(0xaabbccdd));
	printtos();
	printsegaddress();
	printprocstks(-1);
	syscallsummary_start();
	resume(prX = create(prch, 200, 20, "proc X", 1, 'A'));
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();
	return 0;
}
