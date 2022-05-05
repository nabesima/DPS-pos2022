namespace DPS {

#include "getRSS.c"

double peakMemory() { return (double)getPeakRSS()    / (1024 * 1024); }
double usedMemory() { return (double)getCurrentRSS() / (1024 * 1024); }

#include <unistd.h>

double physicalMemory() {
    auto pages = sysconf(_SC_PHYS_PAGES);
    auto page_size = sysconf(_SC_PAGE_SIZE);
    return (double)pages * page_size / (1024 * 1024);
}

}