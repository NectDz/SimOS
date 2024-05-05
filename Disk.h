#ifndef DISK_H
#define DISK_H

#include <vector>
#include <string>
#include <deque>
#include "PCB.h"

struct FileReadRequest
{
    int  PID{0};
    std::string fileName{""};
};

class Disk {
private:
    int diskNumber; // Unique identifier for the disk
    std::deque<FileReadRequest> ioQueue; // Queue for managing I/O requests

public:
    Disk(int num) : diskNumber(num) {} // Constructor to initialize the disk with a disk number

    void addRequest(const FileReadRequest& request) {
        ioQueue.push_back(request);
    }

    void processRequest() {
        if (!ioQueue.empty()) {
            FileReadRequest request = ioQueue.front();
            // Simulate reading the file
            ioQueue.pop_front();
            // Notify completion (this could be a callback or a signal to the process)
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
