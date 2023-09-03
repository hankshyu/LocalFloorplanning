#include <assert.h>
#include <sstream>
#include "monitor.h"

mnt::Monitor::Monitor() : mIterationCounter(-1), mPhaseCounter(-1) {
    this->mClockStartingPoint = std::clock();
    this->mClockCounter = std::clock();
    this->mPhaseCounter = 1;
}

clock_t mnt::Monitor::toggleCounter(){

    clock_t elapsed = clock() - this->mClockCounter;
    this->mClockCounter = clock();

    return elapsed;
}

clock_t mnt::Monitor::getTotalElapsedTime(){
    return clock() - mClockStartingPoint;
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
    std::cout << "Authors: ...." << std::endl;
}

void mnt::Monitor::printPhase(std::string title, int iteration){

    if(iteration != mIterationCounter){
        // Whole new iteration has started over
        assert(iteration == (mIterationCounter +1));
        mIterationCounter = iteration;
        mPhaseCounter = 1;
    }
    
    // assert(this->mPhaseCounter > 0 && this->mPhaseCounter <= 20);
    std::string toPrint = "Iteration " + std::to_string(iteration) + "\tPhase " + mnt::numToTxt[this->mPhaseCounter] + ": " + title;

    std::cout << mnt::PHASE_BORDER << std::endl;
    std::cout << std::left << std::setw(73) << toPrint << "|" << std::right <<std::endl;
    std::cout << mnt::PHASE_BORDER << std::endl;

    this->mPhaseCounter++;

}

void mnt::Monitor::printPhase(std::string title){
    
    // assert(this->mPhaseCounter > 0 && this->mPhaseCounter <= 20);
    std::string toPrint = "Iteration " + std::to_string(mIterationCounter) + "\tPhase " + mnt::numToTxt[this->mPhaseCounter] + ": " + title;

    std::cout << mnt::PHASE_BORDER << std::endl;
    std::cout << std::left << std::setw(73) << toPrint << "|" << std::right <<std::endl;
    std::cout << mnt::PHASE_BORDER << std::endl;

    this->mPhaseCounter++;

}

void mnt::Monitor::printPhaseReport(){
    
    assert(this->mPhaseCounter > 0 && this->mPhaseCounter <= 20);
    std::string toPrint = "Iteration " + std::to_string(mIterationCounter) + "\tPhase " + mnt::numToTxt[this->mPhaseCounter - 1] + " Report";
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << ((double)toggleCounter() / CLOCKS_PER_SEC);
    std::string timeInfo = "Phase duration: "  + ss.str() + " (s)";

    ss.str("");
    double elaspedSeconds =  getElapsedSeconds();
    if(elaspedSeconds > 60){
        int minutes = ((int)elaspedSeconds) / 60;
        elaspedSeconds -= (minutes * 60);
        ss << std::fixed << std::setprecision(2) << elaspedSeconds;
        timeInfo = "Program Time Elapsed: " + std::to_string(minutes) + "(min) " + ss.str() + " (s)";
    }else{
        ss << std::fixed << std::setprecision(2) << elaspedSeconds;
        timeInfo = "Program Time Elapsed: "  + ss.str() + " (s)";

    }


    std::cout << mnt::NORMAL_BORDER << std::endl;
    std::cout << std::left << std::setw(53) << toPrint << "|" << std::right <<std::endl;
    std::cout << std::left << std::setw(53) << timeInfo << "|" << std::right <<std::endl;
    std::cout << mnt::NORMAL_BORDER << std::endl;


}

double mnt::Monitor::getElapsedSeconds(){
    return ((double)getTotalElapsedTime() / CLOCKS_PER_SEC);
}

double mnt::Monitor::getElapsedSeconds(int &minutes, double &seconds){
    double elaspedSeconds = ((double)getTotalElapsedTime() / CLOCKS_PER_SEC);
    double leftSeconds = elaspedSeconds;
    if(elaspedSeconds > 60){
        int leftMinutes = ((int)elaspedSeconds) / 60;
        leftSeconds -= (minutes * 60);

        minutes = leftMinutes;
        seconds = leftSeconds;
    }else{
        minutes = 0;
        seconds = elaspedSeconds;
    }

    return elaspedSeconds;
}

// void mnt::Monitor::printFinalTimeReport(){
//     //TODO
// }





