#ifndef SIM_OS_H
#define SIM_OS_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "Disk.h"
#include "FileReadRequest.h"

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
        PCB newProcess(PID); // Create a new PCB object into another array 
        
        if (currentPID == NO_PROCESS){
            AddProcessToCPU(newProcess);
        }
        else {
            AddProcessToReadyQueue(newProcess);
        }

    }

    void AddProcessToReadyQueue(PCB process){
        process.state = "Ready";

        if (readyQueue.empty()){
            AddProcessToCPU(process);
        }
        else {
            readyQueue.push_back(process.PID);
        }
    }

    void AddProcessToCPU(PCB process){
        process.state = "Running";
        currentPID = process.PID;
    }

    int GetCPU(){
        return currentPID;
    }

 
    void DiskReadRequest( int diskNumber, std::string fileName ){
        // Look up PCB using PID and change the state to waiting

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

    void DiskJobCompleted( int diskNumber ){
        FileReadRequest IoQueue_Process = disks[diskNumber].DiskJobCompleted();
        AddProcessToReadyQueue(IoQueue_Process.PID);
    }

    std::deque<int> GetReadyQueue( ){
        return readyQueue;
    }

    std::vector<Disk> GetDisks( ){
        return disks;
    }

    // Getters
    int getNumberOfDisks() const { return numberOfDisks; }
    

    // Make the private members of the class
    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;
        std::deque<int> readyQueue; 
        std::vector<Disk> disks;
        int currentPID = NO_PROCESS;  
        int lastPID = 0 ;
};


#endif