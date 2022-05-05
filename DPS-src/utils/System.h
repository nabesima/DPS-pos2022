#ifndef _DPS_SYSTEM_H_
#define _DPS_SYSTEM_H_

#include <cstddef>

namespace DPS {

extern double peakMemory();             // Peak used memory in MBytes
extern double usedMemory();             // Current used memory in MBytes
extern double physicalMemory();
static inline double cpuTime(void) ;    // CPU time in seconds.
static inline double realTime(void);    // Wall time in seconds.

}

//
// Implementation of inline functions:
//
#ifdef _WIN32

#include <Windows.h>
static inline double DPS::realTime() {
    LARGE_INTEGER time, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (double)time.QuadPart / freq.QuadPart;
}
static inline double DPS:cpuTime() {
    FILETIME a,b,c,d;
    GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d);
    return
        (double)(d.dwLowDateTime |
        ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
}

#else

#include <time.h>
#include <sys/time.h>
static inline double DPS::realTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000;
}

static inline double DPS::cpuTime() {
    return (double)clock() / CLOCKS_PER_SEC;
}

#endif

#endif
