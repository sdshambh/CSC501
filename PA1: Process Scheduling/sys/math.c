#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "sched.h"
#include "math.h"

double pow(double x, int y){
	//kprintf("entered in power \n");
        int y1;
        double power_result=1.0f;
		if (y == 0)
			return 1.0f; //if power is 0
		else if(x == 0) 
			return 0.0f; //if base is zero
		else if (y < 0)
			{
				return 1.0/pow( x, -y); //for negative powers
			}
		else
			{
				 power_result = pow(x, y/2);
				 power_result *= power_result;
				 if((y%2)!= 0)
				 {
					power_result = power_result * x; 
				 }
				 return power_result; //for all positive numbers
			}
}

double log(double x){
	double i=0;
    double log_result = 0;
	//kprintf("Entering log \n");
	// ln x = (x-1)/x + 1/2 (x-1/x)^2 + 1/3 (x-1/x)^3 ......
	if (x > 0)
	{
		if (x >= 1)
		{
			for(i = 1 ; i <= 20; i++)
			{
				log_result = log_result + (pow(((x-1)/x),i) * (1/i));
			}
			
		}
		//ln x = (1-x) + 1/2 (1-x)^2 - 1/3 (1-x)^3 ......
		else if(x < 1)
		{
			for(i = 1 ; i <= 20; i++)
			{
				log_result = log_result + (pow((1-x),i) * (1/i));
			}
			log_result = log_result * (-1);
		}
		else
		{
			return -1;
		}
		//kprintf("%d \n",(int)log_result);
		return log_result;
	}
	else
	{
		return -1;
	}
	
	
	/*log_result = 1-x;
	for(i=2; i<41; i++){
		log_result = log_result + pow(-1,i)+(pow((1-x),i)/i);
	}

	return 1-log_result;*/
}

double expdev(double lambda) {
    double d;
    do
	{
		d= (double) rand() / RAND_MAX;
    }
	while (d == 0.0);
    return -log(d) / lambda;
}
	