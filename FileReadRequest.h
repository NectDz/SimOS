#ifndef FILE_READ_REQUEST_H
#define FILE_READ_REQUEST_H

#include <string>

struct FileReadRequest {
    int PID{0};
    std::string fileName{""};
};

#endif