int var_sched_type = 0;
int epoch;

void setschedclass(int sched_class){
	var_sched_type = sched_class;
	//kprintf("Set class is %d \n",var_sched_type);
} 
int getschedclass(){
	//kprintf("Get class is %d \n",var_sched_type);
return var_sched_type;
}
