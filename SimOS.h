#ifndef SIM_OS_H
#define SIM_OS_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "Disk.h"
#include "FileReadRequest.h"
#include <unordered_map>

struct MemoryItem
{
    unsigned long long pageNumber;
    unsigned long long frameNumber;
    int PID; // PID of the process using this frame of memory
};
 
using MemoryUsage = std::vector<MemoryItem>;
 
constexpr int NO_PROCESS{ 0 };

class SimOS
{
    public:
    SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize){
        this->numberOfDisks = numberOfDisks;
        this->amountOfRAM = amountOfRAM;
        this->pageSize = pageSize;

        // Calculate the number of frames in RAM
        maxFrames = amountOfRAM / pageSize;

        CreateDisks(numberOfDisks);
    }

    // Creates Number of Disks
    void CreateDisks( int numberOfDisks ){
        int startDiskNumber = 0;

        for (int i = 0; i < numberOfDisks; i++){
            Disk newDisk(startDiskNumber);
            disks.push_back(newDisk);
            startDiskNumber++;
        }
    }

    void NewProcess( ){
        // Create a new PCB object
        int PID = lastPID + 1;
        lastPID = PID; // Update last PID
        PCB newPCB(PID);  // Create a new PCB with initialization
        processTable[PID] = newPCB;  // Store the PCB in the process table

        if (currentPID == NO_PROCESS){
            AddProcessToCPU(newPCB);
        }
        else {
            AddProcessToReadyQueue(newPCB);
        }
    }

    void AddProcessToReadyQueue(PCB process){
        process.state = "Ready";
        processTable[process.PID] = process;
        

        if (readyQueue.empty() && currentPID == NO_PROCESS){
            AddProcessToCPU(process);
        }
        else {
            readyQueue.push_back(process.PID);
        }
    }

    void AddProcessToCPU(PCB process, int timerInterrupt = false){
        // Remove PID from the ready queue
        if (timerInterrupt){
            readyQueue.pop_front();
        }
        
        process.state = "Running";
        currentPID = process.PID;
    }

    int GetCPU(){
        return currentPID;
    }

 
    void DiskReadRequest( int diskNumber, std::string fileName ){
        // Look up PCB using PID and change the state to waiting

        if (currentPID == NO_PROCESS) {
            throw std::logic_error("");
        }
        
        // If Disk number doesnt exist through std::out_of_range exception
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("");
        }

        // Add the process to the IO queue
    
        disks[diskNumber].addRequest(FileReadRequest{currentPID, fileName});

        // Change the currentPID to NO_PROCESS || # Might need to change this to the next process in the ready queue
        currentPID = NO_PROCESS;
    }


    FileReadRequest GetDisk( int diskNumber ){
        if (disks[diskNumber].isQueueEmpty()){
            return FileReadRequest{NO_PROCESS, ""};
        }

        return disks[diskNumber].processRequest();
    }
    
    std::deque<FileReadRequest> GetDiskQueue( int diskNumber ){
        return disks[diskNumber].getIOQueue();
    }

    void DiskJobCompleted(int diskNumber) {
        FileReadRequest IoQueue_Process = disks[diskNumber].DiskJobCompleted();
        PCB process = processTable[IoQueue_Process.PID];
        AddProcessToReadyQueue(process);
    }

    std::deque<int> GetReadyQueue( ){
        return readyQueue;
    }

    std::vector<Disk> GetDisks( ){
        return disks;
    }

    void SimFork() {
        /* # PID Method
        PCB process = processTable[currentPID];
        PCB newProcess = process;
        newProcess.PID = lastPID + 1;
        lastPID = newProcess.PID;
        processTable[newProcess.PID] = newProcess;
        AddProcessToReadyQueue(newProcess, true);
        */

       // PCB Method

        if (currentPID == NO_PROCESS) {
            throw std::logic_error("");
        }
        PCB currentProcess = processTable[currentPID];
        PCB childProcess = currentProcess.forkProcess();
        lastPID = childProcess.PID;

        processTable[childProcess.PID] = childProcess;
        AddProcessToReadyQueue(childProcess);
    }

    void TimerInterrupt(){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("");
        }

        PCB currentProcess = processTable[currentPID];
        AddProcessToReadyQueue(currentProcess);
        PCB nextProcess = readyQueue.front();
        AddProcessToCPU(nextProcess, true);
    }

    void SimExit(){ // Implement Cascading Termination
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("");
        }

        PCB currentProcess = processTable[currentPID];

        PCB parentProcess = processTable[currentProcess.getParentID()];

        auto it = memoryUsage.begin();
        while (it != memoryUsage.end()) {
        if (it->PID == currentPID) {
            memoryUsage.erase(it);
            break;  
        }
            ++it;
        }

        if (this->GetReadyQueue().size() == 0){
            currentProcess.state = "Terminated";
            processTable[currentPID] = currentProcess;
            currentPID = NO_PROCESS;
        } else { 

            if (parentProcess.getState() == "Waiting"){
                currentProcess.state = "Terminated";
                AddProcessToReadyQueue(parentProcess);

                PCB nextProcess = readyQueue.front();
                AddProcessToCPU(nextProcess, true);
            } else {
                currentProcess.state = "Terminated";
                processTable[currentPID] = currentProcess;

                PCB nextProcess = readyQueue.front();
                AddProcessToCPU(nextProcess, true);
            }
        }

    }

    void SimWait(){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("");
        }

        PCB currentProcess = processTable[currentPID];

        if (currentProcess.getChildren().empty()){
            return;
        }

        // Check for Zombies
        std::vector<PCB> childProcesses = currentProcess.getChildren();
        bool zombie = false; 

        for (auto child : childProcesses){
            if (child.getState() == "Terminated"){
                currentProcess.deleteChildProcess(child.PID);
                processTable.erase(child.PID);

                currentProcess.state = "Running";
                processTable[currentPID] = currentProcess;
                zombie = true;
                break;
            }
        }
        
        if (!zombie){
            currentProcess.state = "Waiting";
            processTable[currentPID] = currentProcess;
        }

        PCB nextProcess = readyQueue.front();
        AddProcessToCPU(nextProcess, true);

    }

    void AccessMemoryAddress(unsigned long long address){
        unsigned long long pageNumber = address / pageSize;
        unsigned long long frameNumber = pageNumber % maxFrames;

        memoryUsage.push_back(MemoryItem{pageNumber, frameNumber, currentPID});
    }
    
    void nextProcess(){
        AddProcessToCPU(readyQueue.front(), true);
    }

    MemoryUsage GetMemory(){
        return memoryUsage;
    }

    // Getters
    int getNumberOfDisks() const { return numberOfDisks; }
    

    // Make the private members of the class
    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;

        int maxFrames; 
        MemoryUsage memoryUsage;

        std::deque<int> readyQueue; 
        std::vector<Disk> disks;
        int currentPID = NO_PROCESS;  
        int lastPID = 0 ;

        std::unordered_map<int, PCB> processTable; // Pros and Cons of using a map
};


#endif