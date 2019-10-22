# CSC501
Operating Systems

PA0: Getting Acquainted with XINU
----------------------------------

The objective of this introductory lab is to familiarize you with the process of compiling and running XINU, the tools involved, and the
run-time environment and segment layout.
Implement following XINU functions 
1) long zfunction(long param)
2) void printsegaddress()
3) void printtos()
4) void printprocstks(int priority)
5) void printsyscallsummary()

The TMP folder contains all the files changed and report.

To RUN go to compile directory and do make run followed by make.

PA1: Process Scheduling
------------------------------------------------

Objective: To get familiar with the concepts of process management, including process priorities, scheduling, and context switching.
Implement two schedulers to solve the starvation problem:

1) Exponential Distribution Scheduler:

Ensures fairness by probability distribution and preemptions. 
This scheduler chooses the next process based on a random value that follows the exponential distribution.

2) Linux-Like Scheduler (based loosely on the Linux kernel 2.2):

Ensures fairness by predetermined time quota and epochs (also considering the goodness value giving low priority tasks sufficient fairness to run).
The scheduler chooses the next process based on goodness value, it's time quantum and priority. Important terms: Epoch. Time quantum, Base priority, Goodness Value.

Default schduler is Round Robin.
