#include "SimOS.h"
#include <iostream>

int main(){
    SimOS os(3, 1000000000, 4096);
    std::cout << "Number of disks: " << os.getNumberOfDisks() << std::endl;
    return 0;
}
 