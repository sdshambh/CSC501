#define EXPDISTSCHED 1
#define LINUXSCHED 2
extern int epoch;

void setschedclass(int sched_class); 
int getschedclass();