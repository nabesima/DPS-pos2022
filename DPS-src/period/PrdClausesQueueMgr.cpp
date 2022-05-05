#include <cassert>
#include <stdexcept>

#include "PrdClausesQueueMgr.h"

using namespace DPS;

PrdClausesQueueMgr::PrdClausesQueueMgr(int num_threads) {
    setNumThreads(num_threads);
}

void PrdClausesQueueMgr::setNumThreads(int num_threads) {
    assert(queues.size() == 0);
    for (int i=0; i < num_threads; i++) {
        PrdClausesQueue *mgr = new PrdClausesQueue(i, num_threads);
        if (!mgr) throw std::runtime_error("could not allocate memory for PrdClausesQueue");
        queues.push_back(mgr);
    }
}

PrdClausesQueueMgr::~PrdClausesQueueMgr() {
    for (size_t i=0; i < queues.size(); i++)
        delete queues[i];
    queues.clear();
}

PrdClausesQueue& PrdClausesQueueMgr::get(uint32_t thread_id) const {
    assert(0 <= thread_id);
    assert(thread_id < queues.size());
    return *queues[thread_id];
}

