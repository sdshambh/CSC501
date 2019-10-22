#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
 
extern unsigned long ctr1000;
int tracing;
const char syscalls[27][10] = {"freemem", "chprio", "getpid", "getprio", "gettime", "kill", 
			"receive", "recvclr", "recvtim", "resume", "scount", "sdelete", 
				"send", "setdev", "setnok", "screate", "signal", "signaln", 
					"sleep", "sleep10", "sleep100", "sleep1000", "sreset", "stacktrace",
						"suspend", "unsleep", "wait"};
void syscallsummary_start(){
	int k = 0,l = 0;
	tracing = 1;
	for(k = 0; k < NPROC; k++)
	{
		for(l = 0; l < 27 ; l++)
		{
			proctab[k].systable.execution_time[l] = 0;
			proctab[k].systable.frequency[l] = 0;
		}
	}
		
}
void syscallsummary_stop(){
	 tracing = 0;
}
void printsyscallsummary(){

	int k = 0;
	int p = 0;
	int flag = 0;
	kprintf("\n\nvoid printsyscallsummary()");
	for(k = 0; k < NPROC ; k++){
		
			for(p = 0;  p < 27 ; p++){
			
				//if(((proctab[k].systable.frequency[p] > 0)&&(proctab[k].pstate != PRFREE)) || ((!(proctab[k].pstate != PRFREE)) && (proctab[k].systable.frequency[p] > 0)))
				if(proctab[k].systable.frequency[p] > 0)
				{
					if(flag == 0){
						kprintf("\nProcess [pid:%d]\n", k);
						flag = 1;
					}
					kprintf("\tSyscall: sys_%s, ", syscalls[p]);
					kprintf(" count: %d,", proctab[k].systable.frequency[p]);
					kprintf(" average execution time: %d (ms)\n", (proctab[k].systable.execution_time[p]/proctab[k].systable.frequency[p]));
				}
				
			}
		//}
		flag = 0;
		
	}
}