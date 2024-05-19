// Kevin Granados

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
    std::vector<PCB> children;
    int parentPID = NO_PARENT;
    // Default Constructor
    PCB() : PID(0), state("New"), programCounter(0) {}

    // Constructor
    PCB(int id) : PID(id), state("New"), programCounter(0) {}

    std::vector<PCB>& getChildren() {
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

    PCB forkProcess(int newPID) {
        PCB child = *this; 
        child.PID = newPID;
        child.parentPID = this->PID; 
        child.state = "New";
        children.push_back(child); 
        
        return child;
    }

    int getParentID() const {
        return parentPID;
    }

    std::string getState() const {
        return state;
    }

    void changeChildState(int childPID, const std::string newState) {
        for (PCB &child : children) {
            if (child.PID == childPID) {
                child.state = newState;
                break;
            }
        }
    }

};

#endif
