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
            std::string phaseTimeArr [20];
            clock_t mClockCounter;
            int mPhaseCounter;

        public:
            Monitor();

            void printCopyRight();
            void printPhase(std::string title);
            void printPhaseReport();
            void printFinalTimeReport();

            clock_t toggleCounter();

     
    };

} // namespace MNT

#endif //__MONITOR_H__