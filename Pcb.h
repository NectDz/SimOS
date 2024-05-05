#ifndef PCB_H
#define PCB_H

#include <string>
#include <vector>

class PCB {
public:
    int PID;                        
    std::string state;                      
    int programCounter;             

    // Constructor
    PCB(int id) : PID(id), state("New"), programCounter(0) {}

};

#endif
