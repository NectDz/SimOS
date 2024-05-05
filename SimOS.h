#ifndef SIM_OS_H
#define SIM_OS_H

#include <vector>
#include <string>

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
    }

    // Getters
    int getNumberOfDisks() const { return numberOfDisks; }
    

    // Make the private members of the class
    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;
};


#endif