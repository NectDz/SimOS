// Kevin Granados

#ifndef DISK_H
#define DISK_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "FileReadRequest.h"
#include <iostream>
class Disk {
private:
    int diskNumber;
    std::deque<FileReadRequest> ioQueue; 

public:
    /**
        Parameterized constructor.
            @param   : diskNumber (an int)
                
            @post     : A Disk object is created with diskNumber set to 0 and an empty ioQueue
    */
    Disk(int num) : diskNumber(num), ioQueue() {} 

    /**
            @param   : request (an FileReadRequest object)
                
            @post     : adds the request to the ioQueue
    */
    void addRequest(const FileReadRequest request) {
        ioQueue.push_back(request);
    }

    /**
        @return  : the first FileReadRequest object in the ioQueue
    */
    const FileReadRequest processRequest() {
        return ioQueue.front();
    }

    /**
        @post  : removes the first FileReadRequest object in the ioQueue
    */
    FileReadRequest DiskJobCompleted(){
        FileReadRequest request;
        if (!ioQueue.empty()) {
            request = ioQueue.front();
            ioQueue.pop_front();
        }
        return request;
    }

    /**
        @return  : true if the ioQueue is empty, false otherwise
    */
    bool isQueueEmpty() const {
        return ioQueue.empty();
    }

    /**
        @return  : the diskNumber of the Disk object
    */
    int getDiskNumber() const {
        return diskNumber;
    }

    /**
        @return  : a copy of the ioQueue of the next serving process
    */
    std::deque<FileReadRequest> getIOQueue() const {
        std::deque<FileReadRequest> copy = ioQueue;
        if (!copy.empty()) {
            copy.pop_front();
        }
        return copy;
    }
};

#endif
