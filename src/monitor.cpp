#include <assert.h>
#include <sstream>
#include <stdio.h>
#include <limits>
#include "monitor.h"

mnt::Monitor::Monitor() : mIterationCounter(-1), mPhaseCounter(-1) {
    mClockStartingPoint =  mClockIterationCounter =  mClockCounter = std::clock();
    this->mPhaseCounter = 1;
}

mnt::Monitor::~Monitor(){
    for(runConfig *config : configs){
        delete(config);
    }
}

clock_t mnt::Monitor::toggleCounter(){

    clock_t elapsed = std::clock() - this->mClockCounter;
    this->mClockCounter = std::clock();

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
    std::cout << "Authors: Prof. Iris Jiang, Kevin Chen, Ryan Lin, Hank Hsu" << std::endl;
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
        leftSeconds -= (leftMinutes * 60);

        minutes = leftMinutes;
        seconds = leftSeconds;
    }else{
        minutes = 0;
        seconds = elaspedSeconds;
    }

    return elaspedSeconds;
}

void mnt::Monitor::startIteratrion(){
    mClockIterationCounter = std::clock();
}
double mnt::Monitor::getIterationSeconds(int &minutes, double &seconds){
    
    double elaspedSeconds = ((std::clock() - mIterationCounter) / CLOCKS_PER_SEC);
    double leftSeconds = elaspedSeconds;
    if(elaspedSeconds > 60){
        int leftMinutes = ((int)elaspedSeconds) / 60;
        leftSeconds -= (leftMinutes * 60);

        minutes = leftMinutes;
        seconds = leftSeconds;
    }else{
        minutes = 0;
        seconds = elaspedSeconds;
    }

    return elaspedSeconds;
}

void mnt::Monitor::recordInteration( int iteration, int epoch, double punishmentValue, double toleranceLengthValue, 
            double OBAreaWeight, double OBUtilWeight, double OBAspWeight, double BWUtilWeight, double BWAspWeight,
            int minutes, double seconds,
            bool legaliseSuccess, bool legal,bool fault, double resultHPWL){

    runConfig *cf = new runConfig;

    cf->iteration = iteration;
    cf->epoch = epoch;

    cf->punishmentValue = punishmentValue;
    cf->toleranceLengthValue = toleranceLengthValue;
    cf->OBAreaWeight = OBAreaWeight;
    cf->OBUtilWeight = OBUtilWeight;
    cf->OBAspWeight = OBAspWeight;
    cf->BWUtilWeight = BWUtilWeight;
    cf->BWAspWeight = BWAspWeight;

    cf->minutes = minutes;
    cf->seconds = seconds;

    cf->legaliseSuccess = legaliseSuccess;
    cf->legal = legal;
    cf->fault = fault;
    cf->resultHPWL = resultHPWL;

    configs.push_back(cf);
}

void mnt::Monitor::finalReport(bool legalSolutionFound, double inbestHPWL){
    if(configs.empty()) return;
        bool verifySolutionFound = false;
        double bestHPWL = std::numeric_limits<double>::max();
        for(runConfig *cf : configs){
            printf("[%2d,%2d] (P,T) = (%11.2f, %11.2f)", cf->iteration, cf->epoch, cf->punishmentValue, cf->toleranceLengthValue);
            printf(", (%10.2f, %10.2f, %10.2f, %10.2f, %10.2f)", cf->OBAreaWeight, cf->OBUtilWeight, cf->OBAspWeight, cf->BWUtilWeight, cf->BWAspWeight);
            printf(", Time = %1d:%02.0f", cf->minutes, cf->seconds);
            printf(" >> (Lglise, Lg?, fault, HPWL) = (%1d, %1d, %1d, %12.2f)\n", cf->legaliseSuccess, cf->legal, cf->fault, cf->resultHPWL);
            if(cf->legal) verifySolutionFound = true;
            if(cf->legaliseSuccess && cf->legal){
                if(cf->resultHPWL < bestHPWL) bestHPWL = cf->resultHPWL;
            }
        }

        assert(verifySolutionFound == legalSolutionFound);
        assert(inbestHPWL == bestHPWL);


        if(verifySolutionFound){
            printf("[Success]");
        }else{
            printf("[Fail]");
        }
        int etMin;
        double etSec;
        double elapsedTime = getElapsedSeconds(etMin, etSec);        
        std::cout <<  " Program runtime: " << etMin << "(min) " << etSec <<"(s)";
        if(verifySolutionFound){
            printf("\t best HPWL = %14.2f\n", bestHPWL);
        }

}



