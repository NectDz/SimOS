#ifndef DISK_H
#define DISK_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "FileReadRequest.h"

class Disk {
private:
    int diskNumber;
    std::deque<FileReadRequest> ioQueue; 

public:
    Disk(int num) : diskNumber(num), ioQueue() {} 

    void addRequest(const FileReadRequest request) {
        ioQueue.push_back(request);
    }

    const FileReadRequest processRequest() {
        FileReadRequest request;
        if (!ioQueue.empty()) {
            request = ioQueue.front();
            ioQueue.pop_front();
        }
        return request;
    }

    void DiskJobCompleted(){
        if (!ioQueue.empty()) {
            ioQueue.pop_front();
        }
    }

    bool isQueueEmpty() const {
        return ioQueue.empty();
    }

    int getDiskNumber() const {
        return diskNumber;
    }

    std::deque<FileReadRequest> getIOQueue() const {
        return ioQueue;
    }
};

#endif
