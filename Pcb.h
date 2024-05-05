#ifndef PCB_H
#define PCB_H

#include <string>
#include <vector>

class PCB {
public:
    int PID;                        
    std::string state;              
    int priority;                   
    int programCounter;             

    // Constructor
    PCB(int id, int prio) : PID(id), priority(prio), state("New"), programCounter(0) {}

};

#endif
