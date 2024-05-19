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
#include <algorithm>

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
    /**
        Parameterized constructor.
           @param    : The number of disks (a int)
           @param    : The amount of RAM (a unsigned long long)
           @param    : The pageSize (a unsigned int)

            @post     : Sets the number of disks, amount of RAM and pageSize to the value of the parameters
                It will also generate the max number of frames in RAM
    */
    SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize){
        this->numberOfDisks = numberOfDisks;
        this->amountOfRAM = amountOfRAM;
        this->pageSize = pageSize;

        // Calculate the number of frames in RAM
        maxFrames = amountOfRAM / pageSize;

        CreateDisks(numberOfDisks);
    }

    
    /**
        @post : Creates the number of disks specified by the parameter
            The disks are created with the disk number starting at 0
    */
    void CreateDisks( int numberOfDisks ){
        int startDiskNumber = 0;

        for (int i = 0; i < numberOfDisks; i++){
            Disk newDisk(startDiskNumber);
            disks.push_back(newDisk);
            startDiskNumber++;
        }
    }

    /**
        @post : Creates a new process and adds it to the CPU
            If there is no process currently using the CPU, the new process will be added to the CPU
            If there is a process currently using the CPU, the new process will be added to the ready queue
    */
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

    /**
        @post : Adds the process to the ready queue
            If the ready queue is empty and there is no process currently using the CPU, the process will be added to the CPU
            If the ready queue is not empty, the process will be added to the ready queue
    */

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

    /**
        @post : Adds the process to the CPU
            If the timerInterrupt is true, the process will be removed from the ready queue
            If the timerInterrupt is false, the process will not be removed from the ready queue
    */
    void AddProcessToCPU(PCB &process, int timerInterrupt = false){
        // Remove PID from the ready queue
        if (timerInterrupt && !readyQueue.empty()){
            readyQueue.pop_front();
        }
        
        process.state = "Running";
        processTable[process.PID] = process;

        currentPID = process.PID;
    }
    
    /**
        @post : Adds a disk read request to the specified disk
            If the disk number is out of range, an out_of_range exception will be thrown
            If there is no process currently using the CPU, a logic_error exception will be thrown
            The process will be added to the IO queue
            The next process from the ready queue will be added to the CPU
    */
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

    /**
           @param : The number of disks (a int)

            @post : Returns the disk read request from the specified disk
                If the disk number is out of range, an out_of_range exception will be thrown
                If the disk queue is empty, a FileReadRequest with NO_PROCESS and an empty string will be returned
    */
    FileReadRequest GetDisk( int diskNumber ){
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        if (disks[diskNumber].isQueueEmpty()){
            return FileReadRequest{NO_PROCESS, ""};
        }

        return disks[diskNumber].processRequest();
    }
    
    /**
           @param : The disk Number (a int)

            @post : Returns the disk queue from the next serving process on the disk specified
                If the disk number is out of range, an out_of_range exception will be thrown
    */
    std::deque<FileReadRequest> GetDiskQueue( int diskNumber ){
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        return disks[diskNumber].getIOQueue();
    }

    /**
           @param : The disk Number (a int)

            @post : Removes the next serving process from the disk specified
                If the disk number is out of range, an out_of_range exception will be thrown
                The process will be added to the ready queue
    */
    void DiskJobCompleted(int diskNumber) {
        if (diskNumber >= static_cast<int>(disks.size()) || diskNumber < 0){
            throw std::out_of_range("Disk number is out of range");
        }

        FileReadRequest IoQueue_Process = disks[diskNumber].DiskJobCompleted();
        PCB &process = processTable[IoQueue_Process.PID];
        AddProcessToReadyQueue(process);
    }

    /**
        @post : Simulates a fork system call
            If there is no process currently using the CPU, a logic_error exception will be thrown
            The current process will be forked and added to the ready queue
    */
    void SimFork() {
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        lastPID++;
        PCB childProcess = currentProcess.forkProcess(lastPID); 

        processTable[childProcess.PID] = childProcess;
        childProcess.state = "Ready";  

        AddProcessToReadyQueue(childProcess);
    }

    /**
        @post : Simulates a timer interrupt
            If there is no process currently using the CPU, a logic_error exception will be thrown
            The current process will be added to the ready queue
            The next process from the ready queue will be added to the CPU
    */
    void TimerInterrupt(){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No current process is using the CPU.");
        }

        PCB &currentProcess = processTable[currentPID];
        AddProcessToReadyQueue(currentProcess);
        PCB &nextProcess = processTable[readyQueue.front()];
        AddProcessToCPU(nextProcess, true); // Add New Process Test Case
    }

    /**
        @post : Simulates an exit system call
            If there is no process currently using the CPU, a logic_error exception will be thrown
            The current process will be terminated
            If the parent process is in the waiting state, the current process will be added to the ready queue
            If the parent process is not in the waiting state, the current process will be terminated and if it has children, the children will be removed
    */
    void SimExit(){ 
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

                zombie = true;
                break;
            }
        }
        
        if (!zombie){
            currentProcess.state = "Waiting";

            nextProcess();
        }

    }

    /**
        @post : Adds the next process from the ready queue to the CPU
            If the ready queue is empty, the current process will be set to NO_PROCESS
    */
    void nextProcess() {
        if (!readyQueue.empty()) {
            PCB &nextProcess = processTable[readyQueue.front()];
            readyQueue.pop_front();
            AddProcessToCPU(nextProcess, true);
        } else {
            currentPID = NO_PROCESS;
        }
    }

    /**
        @post : Accesses the memory address
            If there is no process currently using the CPU, a logic_error exception will be thrown
            The memory address will be accessed and added to the memoryUsage and lru list
            If the memory address is already in memory, the memory item will be updated and moved to the front of the lru list
            if the memory is full, the least recently used memory item and lru list will be removed and the new memory item will be added to the memoryUsage and lru list
    */
    void AccessMemoryAddress(unsigned long long address){
        if (currentPID == NO_PROCESS) {
            throw std::logic_error("No process is currently using the CPU.");
        }

        unsigned long long pageNumber = address / pageSize;

        // Check if the memory is full
        bool isFull = memoryUsage.size() == static_cast<size_t>(maxFrames);

        // Check if the page number is in memory
        bool pageNumberFound = isPageAddressInMemory(pageNumber);
        
        // If the page number is not in memory and the memory is not full add the Memory item item to memory and lru list
        if (!pageNumberFound && !isFull) {
            unsigned long long frameNumber = frameCounter; 
            frameCounter++;
            
            MemoryItem newItem{pageNumber, frameNumber, currentPID};

            memoryUsage.push_back({newItem});
            lruList.push_front(newItem);
        } else if (pageNumberFound) { // If the page address is in memory update the lru memory item and move it to the front of the lru list
            MemoryItem &lruItem = lruList.back();
            lruList.pop_back();

            updateMemoryItem(lruItem); // Find LRU and Update it
            
            lruItem.PID = currentPID;
            lruList.push_front(lruItem); // Move to the front of the list since we accessed it
        } else if (isFull) { // If the memory is full remove the least recently used memory item and lru item are removed 
            MemoryItem &lruItem = lruList.back();
            lruList.pop_back(); // Remove the least recently used item

            deleteMemoryItem(lruItem); // Delete the LRU item from memory usage
        }
    }

    /**
        @post : Updates the memory item in the memoryUsage
    */
    void updateMemoryItem(const MemoryItem &itemToUpdate){
        for (auto it = memoryUsage.begin(); it != memoryUsage.end(); ++it) {
            if (it->PID == itemToUpdate.PID && it->pageNumber == itemToUpdate.pageNumber && it->frameNumber == itemToUpdate.frameNumber) {
                it->PID = currentPID;
                break;
            }
        }
    }

    /**
        @post : Deletes the memory item from the memoryUsage
    */
    void deleteMemoryItem(const MemoryItem &itemToDelete){
        auto it = memoryUsage.begin();
        while (it != memoryUsage.end()) {
            if (it->PID == itemToDelete.PID && it->pageNumber == itemToDelete.pageNumber && it->frameNumber == itemToDelete.frameNumber) {
                it = memoryUsage.erase(it);
                break;
            } else {
                ++it;
            }
        }
    }

    /**
        @return : returns true if the page address is in memory, false otherwise
    */
    bool isPageAddressInMemory(unsigned long long address){
        for (auto item : memoryUsage) {
            if (item.PID == currentPID && item.pageNumber == address) {
                return true;
            }
        }

        return false;
    }

    /**
        @post : Terminates the process and remove all of its children 
    */
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

    /* Returns the current running process PID */
    int GetCPU() const{
        return currentPID;
    }

    /* Returns the memoryUsage */
    MemoryUsage GetMemory() const {
        MemoryUsage sortedMemoryUsage = memoryUsage; // Create a copy to sort
        std::sort(sortedMemoryUsage.begin(), sortedMemoryUsage.end(), [](const MemoryItem &a, const MemoryItem &b) {
            return a.frameNumber < b.frameNumber; // Sort by frameNumber
        });
        return sortedMemoryUsage;
    }

    /* Returns the ReadyQueue */
    std::deque<int> GetReadyQueue(){
        return readyQueue;
    }

    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;

        int maxFrames; 
        MemoryUsage memoryUsage;
        std::deque<MemoryItem> lruList;
        int frameCounter = 0;

        std::deque<int> readyQueue; 
        std::vector<Disk> disks;
        int currentPID = NO_PROCESS;  
        int lastPID = 0 ;

        std::unordered_map<int, PCB> processTable; 
};


#endif