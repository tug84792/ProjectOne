# ProjectOne


Harith Siddiqui - 2/12/20

Dr. Eugene Kwatny

CIS 3207

With Project 1: Discrete Event Simulator lab, I learned exactly what the name suggests. I learned (or tried to) how to recreate my own event handler in C++. This, in turn, helped me to better understand how an actual Operating System handles its own respective duties and requests. 

When it came to the actual lab, I started by making the config file and setting appropriate values for each variable. Then, I declared a long list of global variables, so that they can be easily accessed by whatever functions/sections of code that needed them. I used enum to easily access the different states for an event. Speaking of events, I coded that up next, giving it a process ID, time, and enum “typeOfEvent” variables. Then I brought in the seed for random functions. Additionally, I decided to make an event comparison struct. This struct essentially set priorities. If the event times were the same, the earlier event went first. If the times and event types of both processes matched, I made it do the first one as well. Next, I wrote up the necessary queues for the program, which included the priority queue/event queue, the CPU, Disk One, Disk Two, and Network Queues. One thing to note here is the order of the “device” (CPU, Disks, etc). Most of the functions and variables are written in order of them, despite the varying topics. This is another reason why I used C++, due to their built-in queue data structure. Next, I also set up some booleans to see if the device asked for is busy at the time or not. 

Consequently, after opening the config.txt, I store all the values in an array due to its random access ability. Additionally, I am simply more familiar with arrays than linked lists. To start the main program, I set the first and last jobs, pushing them both onto the priority queue (Events queue). The main loop of the program runs, writing as necessary. It also calculates job quantity upon a job’s finish and duration upon entry of a job. It then outputs the results onto the stat.txt file and the runs.txt files. 

The next section has to do with calculating the various data that was asked for, which includes a maximum time duration, average time duration, utilization of the device, maximum queue size, average queue size, and throughput. The maximum values were found through an if comparison, updating the local max through every iteration of the main while loop. The rest of the calculations are all done with simple arithmetic and basic algebra. Following these numbers was the random and probability methods. 

The next list of methods all had to do with the arrival and completion of various Device jobs. For a process arrival, if the CPU was not available, create a cloned process and throw it on to the priority queue, otherwise push it onto the CPU’s queue. Times are constantly updated for these new processes. For the rest of the Device entries, they all followed the same rules and/or steps:
1. There is an if statement that handles a case where the arguments for the function are swapped. 
2. An event is popped from the queue and the current device is switched to “busy mode”.
3. A random time is obtained, updating it to the current task’s own time.
4. A new cloned process is created and placed on the priority queue, and the return values of the functions are used for statistical purposes.

For the CPU completion, it essentially checks the process against all available devices, from the priority queue down to the network. As for the other device completion methods, they also follow a similar structure:
1. A job is popped off, switching the current device to a busy mode. 
2. The CPU availability and queue are checked. The job is placed in the CPU’s queue if it’s busy. 
3. However, if it’s not, the method will push it onto the priority queue. 
4. The last if-statement in each method checks to see if the device’s own queue is empty or not.
5. If it’s not, clone the process, and place it onto the priority queue.
