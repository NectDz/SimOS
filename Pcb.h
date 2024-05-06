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
    std::vector<PCB> children;
    int parentPID;
    // Default Constructor
    PCB() : PID(0), state("New"), programCounter(0), fork(0) {}

    // Constructor
    PCB(int id, int fork=1) : PID(id), state("New"), programCounter(0), fork(fork) {}

    std::vector<PCB> getChildren() const {
        return children;
    }

    PCB forkProcess() {
        PCB child = *this; 
        child.PID = PID + 1;
        child.fork = 0; 
        child.parentPID = PID;
        children.push_back(child); 

        return child;
    }
};

#endif
