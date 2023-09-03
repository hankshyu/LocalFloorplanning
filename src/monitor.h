#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include <time.h>

namespace mnt
{
    const std::string numToTxt [] = {
        "Zero", "One", "Two", "Three", "Four", "Five",
        "Six", "Seven", "Eight", "Nine", "Ten",
        "Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", 
        "Sixteen", "Seventeen", "Eighteen", "Nineteen", "Twenty"
    };

    const std::string PHASE_BORDER  = "==========================================================================";
    const std::string NORMAL_BORDER = "------------------------------------------------------";
    struct runConfig{
        int iteration;
        int epoch;
        
        double punishmentValue;
        double toleranceLengthValue;

        double OBAreaWeight;
        double OBUtilWeight;
        double OBAspWeight;
        double BWUtilWeight;
        double BWAspWeight;

        int minutes;
        double seconds;

        bool legaliseSuccess;
        bool legal;
        bool fault;
        double resultHPWL;

    };

    class Monitor{
    
        private:
            
            clock_t mClockStartingPoint;
            clock_t mClockIterationCounter;
            clock_t mClockCounter;
            
            int mIterationCounter;
            int mPhaseCounter;

            clock_t toggleCounter();
            clock_t getTotalElapsedTime();

            

            std::vector <runConfig*> configs;

        public:
            Monitor();
            ~Monitor();

            void printCopyRight();
            void printPhase(std::string title, int iteration);
            void printPhase(std::string title);
            void printPhaseReport();

            double getElapsedSeconds();
            double getElapsedSeconds(int &minutes, double &seconds);

            void startIteratrion();
            double getIterationSeconds(int &minutes, double &seconds);

            void recordInteration(int iteration, int epoch, double punishmentValue, double toleranceLengthValue, 
                        double OBAreaWeight, double OBUtilWeight, double OBAspWeight, double BWUtilWeight, double BWAspWeight,
                        int minutes, double seconds,
                        bool legaliseSuccess, bool legal, bool fault, double resultHPWL);

            void finalReport(bool legalSolutionFound, double inbestHPWL);
     
    };

} // namespace MNT

#endif //__MONITOR_H__