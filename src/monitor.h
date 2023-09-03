#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <iostream>
#include <iomanip>
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
    class Monitor{
    
        private:
            
            clock_t mClockStartingPoint;
            clock_t mClockCounter;
            
            int mIterationCounter;
            int mPhaseCounter;

            clock_t toggleCounter();
            clock_t getTotalElapsedTime();
        public:
            Monitor();

            void printCopyRight();
            void printPhase(std::string title, int iteration);
            void printPhase(std::string title);
            void printPhaseReport();

            double getElapsedSeconds();
            double getElapsedSeconds(int &minutes, double &seconds);
            // void printFinalTimeReport();


     
    };

} // namespace MNT

#endif //__MONITOR_H__