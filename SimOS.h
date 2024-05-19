// Kevin Granados

#ifndef SIM_OS_H
#define SIM_OS_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "Disk.h"
#include "FileReadRequest.h"
#include <unordered_map>
#include <set> 

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

    void AddProcessToReadyQueue(const PCB &process){
        PCB &readyProcess = processTable[process.PID];
        readyProcess.state = "Ready";
        processTable[readyProcess.PID] = readyProcess;

        if (readyQueue.empty() && currentPID == NO_PROCESS){
            AddProcessToCPU(readyProcess);
        }
        else {
            readyQueue.push_back(readyProcess.PID);
        }
    }


    void AddProcessToCPU(PCB &process, int timerInterrupt = false){
        // Remove PID from the ready queue
        if (timerInterrupt && !readyQueue.empty()){
            readyQueue.pop_front();
        }
        
        process.state = "Running";
        processTable[process.PID] = process;

        currentPID = process.PID;
    }

    int GetCPU() const{
        return currentPID;
    }

 
    void DiskReadRequest( int diskNumber, std::string fileName ){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.s");
        }
        
        // If Disk number doesnt exist through std::out_of_range exception
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        PCB &currentProcess = processTable[currentPID];
        currentProcess.state = "Waiting";  

        // Add the process to the IO queue
    
        disks[diskNumber].addRequest(FileReadRequest{currentPID, fileName});

        // Grab the next process from the ready queue and add it to the CPU
        nextProcess();
    }

    FileReadRequest GetDisk( int diskNumber ){

        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        if (disks[diskNumber].isQueueEmpty()){
            return FileReadRequest{NO_PROCESS, ""};
        }

        return disks[diskNumber].processRequest();
    }
    
    std::deque<FileReadRequest> GetDiskQueue( int diskNumber ){
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        return disks[diskNumber].getIOQueue();
    }

    void DiskJobCompleted(int diskNumber) {
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        FileReadRequest IoQueue_Process = disks[diskNumber].DiskJobCompleted();
        PCB &process = processTable[IoQueue_Process.PID];
        AddProcessToReadyQueue(process);
    }

    std::deque<int> GetReadyQueue(){
        return readyQueue;
    }

    std::vector<Disk> GetDisks( ){
        return disks;
    }

    void SimFork() {
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        lastPID++;
        PCB childProcess = currentProcess.forkProcess(lastPID);  // Ensure this correctly initializes the state

        processTable[childProcess.PID] = childProcess;
        childProcess.state = "Ready";  // Explicitly set the state to Ready

        AddProcessToReadyQueue(childProcess);
    }


    void TimerInterrupt(){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        AddProcessToReadyQueue(currentProcess);
        PCB &nextProcess = processTable[readyQueue.front()];
        AddProcessToCPU(nextProcess, true); // Add New Process Test Case
    }

    int GetCurrentProcessParentID() const {
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        return processTable.at(currentPID).getParentID();
    }

    void SimExit(){ // Implement Cascading Termination
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        PCB &parentProcess = processTable[currentProcess.getParentID()];

        cascadeTermination(currentPID);

        auto it = memoryUsage.begin();
        while (it != memoryUsage.end()) {
            if (it->PID == currentPID) {
                it = memoryUsage.erase(it); 
            } else {
                ++it;
            }
        }

        if (parentProcess.getState() == "Waiting"){
            currentProcess.state = "Terminated";
            parentProcess.changeChildState(currentProcess.PID, "Terminated");

            AddProcessToReadyQueue(parentProcess);
        } else {
            currentProcess.state = "Terminated";
            parentProcess.changeChildState(currentProcess.PID, "Terminated");
        }
        
        nextProcess();

    }

    void SimWait(){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        std::vector<PCB> childProcesses = currentProcess.getChildren();

        // Children size

        if (childProcesses.empty()){
            return;
        }
        
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

            nextProcess();
        }

    }

    void nextProcess() {
        if (!readyQueue.empty()) {
            PCB &nextProcess = processTable[readyQueue.front()];
            readyQueue.pop_front();
            AddProcessToCPU(nextProcess, true);
        } else {
            currentPID = NO_PROCESS;
        }
    }

    void AccessMemoryAddress(unsigned long long address){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No process is currently using the CPU.");
        }

        unsigned long long pageNumber = address / pageSize;
        unsigned long long frameNumber = memoryUsage.size();

        memoryUsage.push_back(MemoryItem{pageNumber, frameNumber, currentPID});
    }

    // Recurive loop through the children of the process

    void cascadeTermination(int pid) {
        PCB &process = processTable[pid];
        std::set<int> childrenToDelete; 

        
        for (PCB &child : process.getChildren()) {
            childrenToDelete.insert(child.PID);
            cascadeTermination(child.PID);
        }

        auto it = memoryUsage.begin();
        while (it != memoryUsage.end()) {
            if (it->PID == pid) {
                it = memoryUsage.erase(it);
            } else {
                ++it;
            }
        }

        for (int child : childrenToDelete) {
            processTable.erase(child);
        }

    }


    // Get Process Table
    std::unordered_map<int, PCB> GetProcessTable() const{
        return processTable;
    }
    

    MemoryUsage GetMemory() const{
        return memoryUsage;
    }

    // Getters
    int getNumberOfDisks() const { return numberOfDisks; }

    int currentProcessParentID() const {
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        return processTable.at(currentPID).getParentID();
    }

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

        std::unordered_map<int, PCB> processTable; 
};


#endif