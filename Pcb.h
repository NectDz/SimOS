#ifndef PCB_H
#define PCB_H

#include <string>
#include <vector>

class PCB {
public:
    int PID;                        
    std::string state;                      
    int programCounter;             
    int fork = 1;

    // Default Constructor
    PCB() : PID(0), state("New"), programCounter(0), fork(0) {}

    // Constructor
    PCB(int id, int fork=1) : PID(id), state("New"), programCounter(0), fork(fork) {}

};

#endif
