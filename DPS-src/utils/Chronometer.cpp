#include <cassert>

#include "Chronometer.h"

using namespace DPS;

void Chronometer::start(ProcName type) {
    system_clock::time_point now = system_clock::now();
    if (stack.size() > 0) {
        ProcTime &pt = stack.back();
        time[pt.type] += now - pt.start;
    }
    count[type]++;
    stack.push_back(ProcTime(type, now));
}

void Chronometer::stop(ProcName type) {
    system_clock::time_point now = system_clock::now();
    ProcTime &pt = stack.back();
    assert(pt.type == type);
    time[pt.type] += now - pt.start;
    stack.pop_back();
    if (stack.size() > 0) stack.back().start = now;
}

void Chronometer::toggle(ProcName from, ProcName to) {
    system_clock::time_point now = system_clock::now();
    // stop 'from' timer
    ProcTime &pt = stack.back();
    assert(pt.type == from);
    time[pt.type] += now - pt.start;
    stack.pop_back();
    if (stack.size() > 0) stack.back().start = now;
    // start 'to' timer
    count[to]++;
    stack.push_back(ProcTime(to, now));
}

void Chronometer::stopAll() {
    system_clock::time_point now = system_clock::now();
    if (stack.size() > 0) {
        ProcTime &pt = stack.back();
        time[pt.type] += now - pt.start;
    }
    stack.clear();
}

void Chronometer::clearAll() {
    for (size_t i=0; i < time.size(); i++)
        time[i] = system_clock::duration(0);
}
