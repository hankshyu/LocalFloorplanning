#include <assert.h>
#include <sstream>
#include "monitor.h"

mnt::Monitor::Monitor(){
    this->mClockCounter = std::clock();
    this->mPhaseCounter = 1;
}

void mnt::Monitor::printCopyRight(){
    std::cout << " ___      _     _          _     " << std::endl;
    std::cout << "|_ _|_ __(_)___| |    __ _| |__  " << std::endl;
    std::cout << " | || '__| / __| |   / _` | '_ \\ " << std::endl;
    std::cout << " | || |  | \\__ \\ |__| (_| | |_) |" << std::endl;
    std::cout << "|___|_|  |_|___/_____\\__,_|_.__/ " << std::endl;
    std::cout << std::endl;
    std::cout << "ICCAD Contest 2023" << std::endl;
    std::cout << "Problem D: Fixed-Outline Floorplanning with Rectilinear Soft Blocks" << std::endl;
}

void mnt::Monitor::printPhase(std::string title){
    
    // assert(this->mPhaseCounter > 0 && this->mPhaseCounter <= 20);
    // std::string toPrint = "Phase " + mnt::numToTxt[this->mPhaseCounter] + ": " + title;
    std::string toPrint = "Phase " + std::to_string(this->mPhaseCounter - 1) + ": " + title;

    std::cout << mnt::PHASE_BORDER << std::endl;
    std::cout << std::left << std::setw(73) << toPrint << "|" << std::right <<std::endl;
    std::cout << mnt::PHASE_BORDER << std::endl;

    this->mPhaseCounter++;

}

void mnt::Monitor::printPhaseReport(){
    
    // assert(this->mPhaseCounter > 0 && this->mPhaseCounter <= 20);
    // std::string toPrint = "Phase " + mnt::numToTxt[this->mPhaseCounter - 1] + " Report";
    std::string toPrint = "Phase " + std::to_string(this->mPhaseCounter - 1) + " Report";
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << ((double)toggleCounter() / CLOCKS_PER_SEC);
    std::string timeInfo = "Time Elapsed: "  + ss.str() + " (s)";
    this->phaseTimeArr[this->mPhaseCounter - 1] = ss.str();


    std::cout << mnt::NORMAL_BORDER << std::endl;
    std::cout << std::left << std::setw(53) << toPrint << "|" << std::right <<std::endl;
    std::cout << std::left << std::setw(53) << timeInfo << "|" << std::right <<std::endl;
    std::cout << mnt::NORMAL_BORDER << std::endl;


}

void mnt::Monitor::printFinalTimeReport(){
    //TODO
}

clock_t mnt::Monitor::toggleCounter(){

    clock_t elapsed = clock() - this->mClockCounter;
    this->mClockCounter = clock();

    return elapsed;
}



