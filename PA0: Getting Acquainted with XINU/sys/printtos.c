#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static unsigned long *esp;
static unsigned long *ebp;

void printtos() {
	int i = 0;
	int num = 5;
	//char k = 'K'; //test variable for question 3
	asm("movl %esp , esp");
	asm("movl %ebp , ebp");
	kprintf("\n\nvoid printtos()");
	kprintf("\nBefore[0x%08x]: 0x%08x", ebp+2, *(ebp+2));	
	kprintf("\nAfter [0x%08x]: 0x%08x", ebp, *(ebp));
	for(i = 1; i < num; i++)
	{
		if((esp+i) < ebp)
		{
			kprintf("\n \telement[0x%08x]: 0x%08x", (esp+i), *(esp+i));
		//kprintf("\n Some Value %08x", k); //test variable print for question 3
		} else {
			break;
		}
	}
}
