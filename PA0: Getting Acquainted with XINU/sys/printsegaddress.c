#include<stdio.h>

extern int end, edata, etext;

void printsegaddress(){	
	
	int i=-1, j=1;
	//printing addresses and values with 8 width and padding with 0 and formatting it to hex -->(0x%08x)
	//formatted printing as expected output
	kprintf("\n\nvoid printsegaddress()");
	kprintf("\nCurrent: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",&etext, etext, &edata, edata, &end, end);
	kprintf("\nPreceeding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",&etext+i, *(&etext+i), &edata+i, *(&edata+i), &end+i, *(&end+i));
	kprintf("\nAfter: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",&etext+j, *(&etext+j), &edata+j, *(&edata+j), &end+j, *(&end+j));
}