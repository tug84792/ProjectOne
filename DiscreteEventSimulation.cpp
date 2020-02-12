#include <queue>
#include <iostream>
#include <list>
#include <cstdlib>
#include <fstream>

// To compile:
// g++ DiscreteEventSimulation.cpp -o harith
using namespace std;

// I used typedef enum to use an index for various states of events so that I can use their
// index values for later purposes
typedef enum {

    jobEnters,
    CpuEnter,
    CpuFinish,
    DiskOneEnter,
    DiskOneFinish,
    DiskTwoEnter,
    DiskTwoFinish,
    NetworkEnter,
    NetworkFinish,
    jobFinishes,
    simulationFinishes

} typeOfEvent;

//Here I have the struct for a process, which includes the PID, time of occurrence,
//and the status of what kind of event it was
struct processStruct{
    int processID;
    int time;
    typeOfEvent status;
};

//To be honest, I don't know what this is, by IDE somehow bugged out and made this function on its own.
//But essentially, it grabs the first item of a future array, and the rest of the program works,
//so I don't think it matters much
static int extracted(const processStruct &item) {
    return item.time;
}

static int extracted(const int *array) {
    int SEED=array[0];
    return SEED;
}

//Here's the struct that I use to compare each processStruct against another processStruct, so I can organize them in the priority queue.
//If the first process processStruct has a lower time it has a higher priority. If they have same time, the one with lower job number goes first,
//and if they have same job number the one with lower status goes first
struct comparisonStruct{

    //I used the built-in for C++
    bool operator()(const processStruct &firstItem, const processStruct &secondItem){

        //Again, with the extraction, I'm not sure how this happened, but I think it still works so:
        //If the times are the same, do the first process first as it has higher priority.
        if(extracted(firstItem) == secondItem.time){
            return firstItem.status > secondItem.status;

        } //Otherwise, if the times match, and the eventType also match, do the first process
            //if it has a smaller PID value
        else if (firstItem.time == secondItem.time && firstItem.status == secondItem.status){
            return extracted(firstItem) > secondItem.processID;

        } //Do the first process as a default
        else {
            return firstItem.time > secondItem.time;
        }
    }
};

//Main priority queue for the various processes,
priority_queue<processStruct, vector<processStruct>, comparisonStruct> qEvents;
//followed by respective queues for different "devices"
queue<processStruct> qCPU;
queue<processStruct> qDiskOne;
queue<processStruct> qDiskTwo;
queue<processStruct> qNetwork;


//Here I have the function signatures for different occurrences of different events in order
//For all the enter functions, they give back an int, which would be duration of the given process
//to use whatever device/queue, which is ultimately used for stats.txt
void procArrives(int min, int max);

int entersCpu(int min, int max);
void completesCpu(int probability);

int entersDiskOne(int min, int max);
void completesDiskOne();

int entersDiskTwo(int min, int max);
void completesDiskTwo();

int networkEnter(int min, int max);
void completesNetwork();

//function signatures for my random number and probability function
int randomNumberGenerator(int lowerEnd, int higherEnd);
bool selectionProbability(int percentChance);

//Here I have boolean values for the states of each of the devices, to see if I can use them or not later on
bool CpuNotAvailable=false;
bool DiskOneNotAvailable=false;
bool DiskTwoNotAvailable=false;
bool NetworkNotAvailable=false;

int main(){

    //Here I open pre-made config file
    ifstream fileToRead("config.txt");

    //Used for indexing objects in the file, and made an array out of the objects in the file
    int index=0, readValue, configureTextArray[14];

    string text;

    //I use a while loop to get each number in a line, store it into the array form before
    while (fileToRead >> text >> readValue){
        configureTextArray[index]=readValue;
        index++;
    }

    //Basically retyping out the file in a way, to match up everything
    int SEED = extracted(configureTextArray);
    int INIT_TIME=configureTextArray[1];
    int FIN_TIME=configureTextArray[2];
    int ARRIVE_MIN=configureTextArray[3];
    int ARRIVE_MAX=configureTextArray[4];
    int QUIT_PROB=configureTextArray[5];
    int CPU_MIN=configureTextArray[6];
    int CPU_MAX=configureTextArray[7];
    int DISK1_MIN=configureTextArray[8];
    int DISK1_MAX=configureTextArray[9];
    int DISK2_MIN=configureTextArray[10];
    int DISK2_MAX=configureTextArray[11];
    int NETWORK_MIN=configureTextArray[12];
    int NETWORK_MAX=configureTextArray[13];

    //Here I open log.txt to display the previous file's contents
    ofstream writingToLog;
    writingToLog.open("log.txt");
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "SEED: " << SEED << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Simulation begins at time:  " << INIT_TIME << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Simulation Completion time is " << FIN_TIME << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum job arrival is: " << ARRIVE_MIN << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum job arrival is: " << ARRIVE_MAX << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "The probability of quit is: " << QUIT_PROB << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum CPU processing is: " << CPU_MIN << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for maximum CPU processing is: " << CPU_MAX << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum Disk One processing is: " << DISK1_MIN << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for maximum Disk One processing is: " << DISK1_MAX << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum Disk Two processing is: " << DISK2_MIN << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for maximum Disk Two processing is: " << DISK2_MAX << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for minimum Network processing is: " << NETWORK_MIN << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;
    writingToLog << "Time for maximum Network processing is: " << NETWORK_MAX << endl;
    writingToLog << "#------------------------------------------------------------------------------#" << endl;

    //Here I initialize many variables, all in the following order of devices:
    //Event, CPU, Disk One, Disk Two, Network
    //Then within that, MaxLength, TotalLength, MaxTimeLength, NumOfJobs, timeFor,
    //averageSize, totalTime, averageTime, utilizationFor, averageSize,throughoutFor
    int qEventMaxLength=0, qEventTotalLength=0;

    int qCpuMaxLength=0, qCPUTotalLength=0, CPUMaxTimeLength=0, NumOfCPUJobs=0, timeForCPU=0;
    double qAverageEventSize=0, CPUTotalTime=(double)CPUMaxTimeLength, averageTimeForCPU=0, utilForCPU=0, qAverageCPUSize=0, throughputForCPU=0;

    int qDiskOneMaxLength=0, qDiskOneTotalLength=0, DiskOneMaxTimeLength=0, NumOfDiskOneJobs=0, timeForDiskOne=0;
    double DiskOneTotalTime=(double)DiskOneMaxTimeLength, averageTimeForDiskOne=0, utilForDiskOne=0, qAverageDiskOneSize=0, throughputForDiskOne=0;

    int qDiskTwoMaxLength=0, qDiskTwoTotalLength=0, DiskTwoMaxTimeLength=0, NumOfDiskTwoJobs=0, timeForDiskTwo=0;
    double DiskTwoTotalTime=(double)DiskTwoMaxTimeLength, averageTimeForDiskTwo=0, utilForDiskTwo=0, qAverageDiskTwoSize=0, throughputForDiskTwo=0;

    int qNetworkMaxLength=0, qNetworkTotalLength=0, NetworkMaxTimeLength=0, NumOfNetworkJobs=0, timeForNetwork=0;
    double NetworkTotalTime=(double)NetworkMaxTimeLength, averageTimeForNetwork=0, utilForNetwork=0, qAverageNetworkSize=0, throughputForNetwork=0;


    //Starting up srand with the SEED value given from configure
    srand(SEED);
    //same case here
    int startTime = INIT_TIME;

    //Here I create a process for finishing the program at the given FIN_TIME
    processStruct simulationCompletition= {0, FIN_TIME, simulationFinishes };
    //creating first job arrival processStruct
    processStruct firstArrivingJob={1, startTime, jobEnters};
    //Push both onto a priority queue
    qEvents.push(firstArrivingJob), qEvents.push(simulationCompletition);

    //Counter variable to calculate number of iterations
    int loopCounter=0;
    //Here is a while loop provided by TA to run main program.
    while(!qEvents.empty() && (startTime < FIN_TIME)){
        loopCounter++;
        processStruct jobCurrentlyRunning = qEvents.top();

        //Start writing to log file
        writingToLog << "At this time: " << jobCurrentlyRunning.time << ", a job with PID: " << jobCurrentlyRunning.processID << " "; //writing to file the startTime and job number

        //Used a switch statement, again shown by TA, to run program. Depending on the process, it will
        //run the appropriate case
        switch (jobCurrentlyRunning.status){

            //For all cases, I either update number of jobs, or the total time, depending on the Enter/Finish
            case jobEnters:
                writingToLog << "has arrived." << endl;
                procArrives(ARRIVE_MIN, ARRIVE_MAX);
                break;

            case CpuEnter:
                writingToLog << "has entered CPU." << endl;
                timeForCPU = entersCpu(CPU_MIN, CPU_MAX);
                CPUTotalTime+=timeForCPU;
                break;

            case CpuFinish:
                writingToLog << "completed at the CPU." << endl;
                completesCpu(QUIT_PROB);
                NumOfCPUJobs++;
                break;

            case DiskOneEnter:
                writingToLog << "has entered Disk One." << endl;
                timeForDiskOne = entersDiskOne(DISK1_MIN, DISK1_MAX);
                DiskOneTotalTime+=timeForDiskOne;
                break;

            case DiskOneFinish:
                writingToLog << "has completed tasks at Disk One." << endl;
                completesDiskOne();
                NumOfDiskOneJobs++;
                break;

            case DiskTwoEnter:
                writingToLog << "has entered Disk Two." << endl;
                timeForDiskTwo= entersDiskTwo(DISK2_MIN, DISK2_MAX);
                DiskTwoTotalTime+=timeForDiskTwo;
                break;

            case DiskTwoFinish:
                writingToLog << "has completed tasks at Disk Two." << endl;
                completesDiskTwo();
                NumOfDiskTwoJobs++;
                break;

            case NetworkEnter:
                writingToLog << "has entered the Network." << endl;
                timeForNetwork=networkEnter(NETWORK_MIN, NETWORK_MAX);
                NetworkTotalTime+=timeForNetwork;
                break;

            case NetworkFinish:
                writingToLog << "has completed tasks at the Network." << endl;
                completesNetwork();
                NumOfNetworkJobs++;
                break;

            case jobFinishes:
                writingToLog << "Exiting the program..." << endl;
                //Here, I pop the job off the pq
                qEvents.pop();
                break;

            case simulationFinishes:
                //Wrote a goodbye message
                writingToLog << " " << endl;
                writingToLog << "#------------------------------------------------------------------------------#" << endl;
                writingToLog << "# Thank you for running this program! #" << endl;
                writingToLog << "# Simulation has now ended. Check the Statistics File, then run again :) #" << endl;
                writingToLog << "#------------------------------------------------------------------------------#" << endl;

                //Closed the log.txt file
                writingToLog.close();

                //Opened the stat.txt file to write to it
                ofstream writingToStats;
                writingToStats.open("stats.txt");

                writingToStats << "                          WELCOME TO THE STATS FILE!                            " << endl;
                writingToStats << "####################### Here are the Max Queue size values #####################" << endl;
                writingToStats << "# Event Queue: " << qEventMaxLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "# CPU Queue: " << qCpuMaxLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "# Disk One Queue: " << qDiskOneMaxLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "# Disk Two Queue: " << qDiskTwoMaxLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "# Network Queue: " << qNetworkMaxLength << endl;
                writingToStats << "################################################################################" << endl;
                writingToStats << "#################### Here are the Average Queue size values ####################" << endl;
                writingToStats << "Event Queue:  " << qAverageEventSize << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "CPU Queue " << qAverageCPUSize << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk One Queue: " << qAverageDiskOneSize << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk Two Queue: " << qAverageDiskTwoSize << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Network Queue: " << qAverageNetworkSize << endl;
                writingToStats << "################################################################################" << endl;
                writingToStats << "################## Here are the Max Component Response Values ##################" << endl;
                writingToStats << "CPU Time: " << CPUMaxTimeLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk One Time: " << DiskOneMaxTimeLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk Two Time: " << DiskTwoMaxTimeLength << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Network Time: " << NetworkMaxTimeLength << endl;
                writingToStats << "################################################################################" << endl;
                writingToStats << "################ Here are the Average Component Response Values ################" << endl;
                writingToStats << "CPU Time: " << averageTimeForCPU << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk One Time: " << averageTimeForDiskOne << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk Two Time: " << averageTimeForDiskTwo << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Network Time: " << averageTimeForNetwork << endl;
                writingToStats << "################################################################################" << endl;
                writingToStats << "################# Here are the Component Throughput Values #####################" << endl;
                writingToStats << "CPU: " << throughputForCPU << " jobs per unit of startTime." << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk One: " << throughputForDiskOne << " jobs per unit of startTime." << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk Two: " << throughputForDiskTwo << " jobs per unit of startTime." << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Network: " << throughputForNetwork << " jobs per unit of startTime." << endl;
                writingToStats << "################################################################################" << endl;
                writingToStats << "################# Here are the Component Utilization Values ####################" << endl;
                writingToStats << "CPU: " << utilForCPU << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk One: " << utilForDiskOne << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Disk Two: " << utilForDiskTwo << endl;
                writingToStats << "#------------------------------------------------------------------------------#" << endl;
                writingToStats << "Network: " << utilForNetwork << endl;
                writingToStats << "################################################################################" << endl;

                //Finally, I close the stat.txt file, and a goodbye message
                writingToStats.close();
                cout<<"Event simulation is now over! Thank you for running. You're more than welcome to run again :)"<<endl;
                exit(0);
        }
        //From here on out, I find the desired calculations. This all happens after each while loop iteration.
        //If statements to compare maxes, calculates averages for time, utilization, max q size,
        //max q length, average q size, throughput, and will all update accordingly.
        //They are in the same normal order:
        //Events --> CPU --> Disk One --> Disk Two -- > Network

        if(qEvents.size() > qEventMaxLength) {
            qEventMaxLength = qEvents.size();
        }
        qEventTotalLength+=qEvents.size();
        qAverageEventSize= ((double)qEventTotalLength / loopCounter);


        if(timeForCPU > CPUMaxTimeLength) {
            CPUMaxTimeLength = timeForCPU;
        }
        averageTimeForCPU= (CPUTotalTime / NumOfCPUJobs);
        utilForCPU= CPUTotalTime / FIN_TIME;
        if(qCPU.size() > qCpuMaxLength) {
            qCpuMaxLength = qCPU.size();
        }
        qCPUTotalLength+=qCPU.size();
        qAverageCPUSize= ((double)qCPUTotalLength / loopCounter);
        throughputForCPU= ((double)NumOfCPUJobs / FIN_TIME);



        if (timeForDiskOne > DiskOneMaxTimeLength) {
            DiskOneMaxTimeLength = timeForDiskOne;
        }
        averageTimeForDiskOne= (DiskOneTotalTime / NumOfDiskOneJobs);
        utilForDiskOne= DiskOneTotalTime / FIN_TIME;
        if(qDiskOne.size() > qDiskOneMaxLength) {
            qDiskOneMaxLength = qDiskOne.size();
        }
        qDiskOneTotalLength+=qDiskOne.size();
        qAverageDiskOneSize= ((double)qDiskOneTotalLength / loopCounter);
        throughputForDiskOne= ((double)NumOfDiskOneJobs / FIN_TIME);



        if(timeForDiskTwo > DiskTwoMaxTimeLength) {
            DiskTwoMaxTimeLength = timeForDiskTwo;
        }
        averageTimeForDiskTwo= (DiskTwoTotalTime / NumOfDiskTwoJobs);
        utilForDiskTwo= DiskTwoTotalTime / FIN_TIME;
        if(qDiskTwo.size() > qDiskTwoMaxLength) {
            qDiskTwoMaxLength = qDiskTwo.size();
        }
        qDiskTwoTotalLength+=qDiskTwo.size();
        qAverageDiskTwoSize= ((double)qDiskTwoTotalLength / loopCounter);
        throughputForDiskTwo= ((double)NumOfDiskTwoJobs / FIN_TIME);



        if (timeForNetwork > NetworkMaxTimeLength) {
            NetworkMaxTimeLength = timeForNetwork;
        }
        averageTimeForNetwork= (NetworkTotalTime / NumOfNetworkJobs);
        utilForNetwork= NetworkTotalTime / FIN_TIME;
        if(qNetwork.size() > qNetworkMaxLength) {
            qNetworkMaxLength = qNetwork.size();
        }
        qNetworkTotalLength+=qNetwork.size();
        qAverageNetworkSize= ((double)qNetworkTotalLength / loopCounter);
        throughputForNetwork= ((double)NumOfNetworkJobs / FIN_TIME);
    }// End of main While loop
}//End of Main Function

//Here I have my probability function, generating between 1% and 100%,
//then further filtering for assurance.
bool selectionProbability(int percentChance){
    //Here, I do some more error handling, making sure the chances are between 0 and 100
    if (percentChance < 0 || percentChance > 100){
        cout<<"Chance probability is out of range"<<endl;
        return -1;
    } else {
        int localVariable = randomNumberGenerator(0, 100);
        return localVariable <= percentChance;
    }
}

//Here I have the Random Number Generator
//if lowerEnd is greater than higherEnd, it switches the numbers
//returns a random integer between the lowerEnd and the higherEnd given
int randomNumberGenerator(int lowerEnd, int higherEnd){
    //Here I filter out the mistake of having the arguments switched
    if (higherEnd < lowerEnd){
        int localVariable = lowerEnd;
        lowerEnd = higherEnd;
        higherEnd = localVariable;
    }
    return lowerEnd + rand() % (higherEnd + 1 - lowerEnd);
}

void procArrives(int min, int max){

    //Error handling
    if (min < max){

        int localVariable = min;
        min = max;
        max = localVariable;
    }
    //The process's event is taken form the top of qEvents
    processStruct currentlyExecutingJob=qEvents.top();
    int index=qEvents.top().processID;
    qEvents.pop();

    //Here I check if the CPU is free and empty, and if it is, create another process that's basically
    //a clone of the process that was passed.
    if (!CpuNotAvailable && qCPU.empty()){
        processStruct newJob={index, currentlyExecutingJob.time, CpuEnter};
        qEvents.push(newJob);
    }
        //Otherwise, it is placed on the CPU's queue
    else if(CpuNotAvailable || !qCPU.empty()){

        qCPU.push(currentlyExecutingJob);
    }

    //new time is generated, then updates to time of current process
    int timeOfArrival= randomNumberGenerator(min, max);
    int jobTime= currentlyExecutingJob.time + timeOfArrival;

    //Then, I create a new job with a PID following the PID of the current process with that updated time,
    //then place it oin the priority queue
    processStruct newJobArrives={++index, jobTime, jobEnters};
    qEvents.push(newJobArrives);
}

//For all the "enter" type of functions, I apply the same logic (and in same device order):
// 1. I have an error handling if statement.
// 2. Event is popped off and the Device is changed to show as busy
// 3. Random time signifying event duration is grabbed here, then updated/added to the currently handled process's time.
// 4. A new process is created with info of current process, then placed on the queue
// 5. The return values are ultimately used for info for stats.txt

int entersCpu(int min, int max) {
    //Error handling
    if (min < max){
        int localVariable = min;
        min = max;
        max = localVariable;
    }
    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    CpuNotAvailable=true;

    int timeOfCompletion= randomNumberGenerator(min, max);
    int updatedTime= currentlyExecutingJob.time + timeOfCompletion;

    //Here I create a new process
    processStruct newJobArrives={currentlyExecutingJob.processID, updatedTime, CpuFinish};
    qEvents.push(newJobArrives);

    //This return value is used for stats.txt
    return timeOfCompletion;
}
int entersDiskOne(int min, int max){

    //Error handling
    if (min < max){
        int localVariable = min;
        min = max;
        max = localVariable;
    }

    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    DiskOneNotAvailable=true;

    int timeOfCompletion= randomNumberGenerator(min, max);
    int updatedTime= currentlyExecutingJob.time + timeOfCompletion;

    processStruct newJobArrives= {currentlyExecutingJob.processID, updatedTime, DiskOneFinish };
    qEvents.push(newJobArrives);

    return timeOfCompletion;
}

int entersDiskTwo(int min, int max){
    //Error handling
    if (min < max){
        int localVariable = min;
        min = max;
        max = localVariable;
    }

    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    DiskTwoNotAvailable=true;

    int timeOfCompletion= randomNumberGenerator(min, max);
    int updatedTime= currentlyExecutingJob.time + timeOfCompletion;

    processStruct newJobArrives= {currentlyExecutingJob.processID, updatedTime, DiskTwoFinish };
    qEvents.push(newJobArrives);

    return timeOfCompletion;
}

int networkEnter(int min, int max){
    //Error handling
    if (min < max){
        int localVariable = min;
        min = max;
        max = localVariable;
    }

    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    NetworkNotAvailable=true;

    int timeOfCompletion= randomNumberGenerator(min, max);
    int updatedTime= currentlyExecutingJob.time + timeOfCompletion;

    processStruct newJobArrives= {currentlyExecutingJob.processID, updatedTime, NetworkFinish };
    qEvents.push(newJobArrives);

    return timeOfCompletion;
}

void completesCpu(int probability){

    //Event is popped off, and CPU switches to busy mode
    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    CpuNotAvailable=false;

    //This if statement calls upon a boolean function, which depending on T/F, it will send to the Disk or not.
    if(selectionProbability(probability)){

        //If it's true, push the job onto the pq
        processStruct jobDeparture={currentlyExecutingJob.processID, currentlyExecutingJob.time, jobFinishes};
        qEvents.push(jobDeparture);
    } //Otherwise, check other devices in the usual order
        //Check Disk One, and apply the same logic
    else if(!DiskOneNotAvailable && qDiskOne.empty()){

        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, DiskOneEnter};
        qEvents.push(newJobArrives);
    }
        //Check Disk Two, and apply the same logic
    else if(!DiskTwoNotAvailable && qDiskTwo.empty()){
        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, DiskTwoEnter};
        qEvents.push(newJobArrives);

    }
        //Check Network, and apply the same logic
    else if(!NetworkNotAvailable && qNetwork.empty()){
        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, NetworkEnter};
        qEvents.push(newJobArrives);
    }
        //Use the next couple of else-if statements to check for the shorter queue, and place it into that one.
    else if(qDiskOne.size() > qDiskTwo.size()) {
        qDiskTwo.push(currentlyExecutingJob);
    }
    else if (qDiskOne.size() < qDiskTwo.size()) {
        qDiskOne.push(currentlyExecutingJob);
    }
    else if((qNetwork.size() < qDiskOne.size()) && (qNetwork.size() < qDiskTwo.size())) {
        qNetwork.push(currentlyExecutingJob);
    }
        //Else, just put it onto the network
    else {
        qNetwork.push(currentlyExecutingJob);
    }
//EDIT CODE HEREEEE

    //Else, if the CPU is not busy at the moment, have it pop off the current job, create a brand new CpuEnter kind
    //of job, and then place it onto the pq.
    if(!qCPU.empty()) {
        processStruct aJob=qCPU.front();
        qCPU.pop();

        processStruct newJobArrives={aJob.processID, currentlyExecutingJob.time, CpuEnter};
        qEvents.push(newJobArrives);
    }
}

//For the rest of the "complete functions", I follow a similar approach as before:
// 1. Event is popped off, and Device switches to busy mode
// 2. The CPU is double checked for availability: if it's busy and its queue isn't empty, place it on the CPU queue
// 3. But if it's not busy and its queue is empty, create a new job and give it the current job's info.
// 4. A new process is created with info of current process, then placed on the priority queue
// 5. The last if statement checks to see if a Device's queue isn't empty
// 6. If it's not, the job is popped, a brand new process is created with the current job's info
// 7. That new job is now placed onto the priority queue.

void completesDiskOne(){
    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    DiskOneNotAvailable=false;

    if(CpuNotAvailable || !qCPU.empty()){
        qCPU.push(currentlyExecutingJob);

    }
    else if(!CpuNotAvailable && qCPU.empty()){
        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, CpuEnter };
        qEvents.push(newJobArrives);
    }
    if(!qDiskOne.empty()){
        processStruct aJob=qDiskOne.front();
        qDiskOne.pop();

        processStruct newJobArrives={aJob.processID, currentlyExecutingJob.time, DiskOneEnter};
        qEvents.push(newJobArrives);
    }
}
void completesDiskTwo(){

    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    DiskTwoNotAvailable=false;

    if(CpuNotAvailable || !qCPU.empty()){
        qCPU.push(currentlyExecutingJob);
    }
    else if(!CpuNotAvailable && qCPU.empty()){
        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, CpuEnter };
        qEvents.push(newJobArrives);
    }
    if(!qDiskTwo.empty()){
        processStruct aJob=qDiskTwo.front();
        qDiskTwo.pop();

        processStruct newJobArrives={aJob.processID, currentlyExecutingJob.time, DiskTwoEnter };
        qEvents.push(newJobArrives);
    }
}

void completesNetwork(){

    processStruct currentlyExecutingJob=qEvents.top();
    qEvents.pop();
    NetworkNotAvailable=false;

    if(CpuNotAvailable || !qCPU.empty()){
        qCPU.push(currentlyExecutingJob);
    }
    else if(!CpuNotAvailable && qCPU.empty()){
        processStruct newJobArrives={currentlyExecutingJob.processID, currentlyExecutingJob.time, CpuEnter };
        qEvents.push(newJobArrives);
    }
    if(!qNetwork.empty()){
        processStruct aJob=qNetwork.front();
        qNetwork.pop();

        processStruct newJobArrives={aJob.processID, currentlyExecutingJob.time, NetworkEnter };
        qEvents.push(newJobArrives);
    }
}