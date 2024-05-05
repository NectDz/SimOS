#ifndef SIM_OS_H
#define SIM_OS_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"

struct FileReadRequest
{
    int  PID{0};
    std::string fileName{""};
};
 
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

    void CreateDisks( int numberOfDisks ){
        // Create a number of disks
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
        readyQueue.push_back(process.PID);
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
        ioQueue.push_back(currentPID);

        // Change the currentPID to NO_PROCESS
        currentPID = NO_PROCESS;
    }

    FileReadRequest GetDisk( int diskNumber ){
        // Implement looking by diskNumber
        FileReadRequest request;
        request.PID = ioQueue.front();
        request.fileName = "file1.txt";
        return request;
    }

    std::deque<int> GetReadyQueue( ){
        return readyQueue;
    }

    // Getters
    int getNumberOfDisks() const { return numberOfDisks; }
    

    // Make the private members of the class
    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;
        std::deque<int> readyQueue; 
        int currentPID;  
        int lastPID = 0 ;
};


#endif