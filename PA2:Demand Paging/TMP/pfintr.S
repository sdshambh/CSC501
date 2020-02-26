/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
			/*store error code in a global variable*/
			popl pferrcode
			/*store flag register*/
			pushfl
			/*clear all interrupts*/
			cli
			/*save all general registers*/
			pushal
			/*call to page fault handling*/
			call pfint
			/*restore general registers*/
			popal
			/*restore flag register*/
			popfl
			iret
