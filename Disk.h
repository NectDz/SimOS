#ifndef DISK_H
#define DISK_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"
#include "SimOS.h"

class Disk {
private:
    int diskNumber;
    std::deque<FileReadRequest> ioQueue; 

public:
    Disk(int num) : diskNumber(num) {} 

    void addRequest(const FileReadRequest& request) {
        ioQueue.push_back(request);
    }

    void processRequest() {
        if (!ioQueue.empty()) {
            FileReadRequest request = ioQueue.front();
            ioQueue.pop_front();
        }
    }

    bool isQueueEmpty() const {
        return ioQueue.empty();
    }

    int getDiskNumber() const {
        return diskNumber;
    }
};

#endif
