#ifndef _DPS_CHRONOMETER_H_
#define _DPS_CHRONOMETER_H_

#include <chrono>
#include <vector>
#include <cstdint>

using namespace std::chrono;

namespace DPS {

enum ProcName {
    RunningTime,
    WaitingTime,
    ExchangingTime,
    PeriodUpdateTime,
    // # of types
    ProcTypes
};

class Chronometer {
private:
    struct ProcTime {
        ProcName                    type;
        system_clock::time_point    start;
        ProcTime(ProcName t, system_clock::time_point s) : type(t), start(s) {}
    };
    std::vector<system_clock::duration> time;
    std::vector<uint64_t>               count;
    std::vector<ProcTime>               stack;

public:
    Chronometer() {
        time.resize(ProcTypes);
        count.resize(ProcTypes);
    };

    void     start   (ProcName type);
    void     stop    (ProcName type);
    void     toggle  (ProcName from, ProcName to);
    void     clear   (ProcName type) { time[type] = system_clock::duration(0); }
    void     clearAll();
    void     stopAll ();
    double   getTime (ProcName type) { return static_cast<double>(duration_cast<nanoseconds>(time[type]).count()) / 1000000000; }
    uint64_t getCount(ProcName type) { return count[type]; }
};

}

#endif

