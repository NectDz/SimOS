#ifndef PCB_H
#define PCB_H

#include <string>
#include <vector>

constexpr int NO_PARENT{ 0 };

class PCB {
public:
    int PID;                        
    std::string state;                      
    int programCounter;             
    int fork = 1;
    std::vector<PCB> children;
    int parentPID = NO_PARENT;
    // Default Constructor
    PCB() : PID(0), state("New"), programCounter(0), fork(0) {}

    // Constructor
    PCB(int id, int fork=1) : PID(id), state("New"), programCounter(0), fork(fork) {}

    std::vector<PCB> getChildren() const {
        return children;
    }

    void deleteChildProcess(int PID) {
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i].PID == PID) {
                children.erase(children.begin() + i);
                break;
            }
        }
    }

    PCB forkProcess() {
        PCB child = *this; 
        child.PID = PID + 1;
        child.fork = 0; 
        child.parentPID = PID;
        children.push_back(child); 

        return child;
    }

    int getParentID() const {
        return parentPID;
    }

    std::string getState() const {
        return state;
    }

};

#endif
