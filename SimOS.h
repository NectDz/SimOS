#ifndef SIM_OS_H
#define SIM_OS_H


class SimOS
{
    public:
    SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize){
        this->numberOfDisks = numberOfDisks;
        this->amountOfRAM = amountOfRAM;
        this->pageSize = pageSize;
    }

    // Make the private members of the class
    private:
        int numberOfDisks;
        unsigned long long amountOfRAM;
        unsigned int pageSize;
}



#endif