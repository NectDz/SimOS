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

    /**
        Default constructor.
            @post     : A PCB object is created with PID set to 0 and state set to "New"
    */
    PCB() : PID(0), state("New") {}

    /**
        Parameterized constructor.
           @param    : the id of the PCB (a int)

            @post     : A PCB object is created with the given id, state is set to "New"
    */

    PCB(int id) : PID(id), state("New") {}

    /**
      @return  : the children vector of the current PCB object
    */
    std::vector<PCB>& getChildren() {
        return children;
    }

    /**
        @param    : the id of the child process (a int)

        @post  : deletes the child process with the given id from the children vector
    */
    void deleteChildProcess(int PID) {
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i].PID == PID) {
                children.erase(children.begin() + i);
                break;
            }
        }
    }

    /**
        @param    : the id of the forked child process (a int)

        @post : creates a new PCB object with the given id and adds it to the children vector
    */
    PCB forkProcess(int newPID) {
        PCB child = *this; 
        child.PID = newPID;
        child.parentPID = this->PID; 
        child.state = "New";
        children.push_back(child); 
        
        return child;
    }

    /**
        @return  : the id of the parent process
    */
    int getParentID() const {
        return parentPID;
    }

    /**
        @return  : the state of the PCB object
    */
    std::string getState() const {
        return state;
    }

    /**
        @param    : the child PID (a int)
        @param    : the new state of the PCB object (a string)

        @post  : sets the state of the PCB object to the given state
    */
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
