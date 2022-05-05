#ifndef _DPS_PRD_CLAUSES_QUEUE_MGR_H_
#define _DPS_PRD_CLAUSES_QUEUE_MGR_H_

#include "PrdClausesQueue.h"

namespace DPS {

// A set of clauses acquired by threads
class PrdClausesQueueMgr {
private:
    std::vector<PrdClausesQueue *> queues;

public:
    PrdClausesQueueMgr(int num_threads);
    ~PrdClausesQueueMgr();

    void setNumThreads(int num_threads);
    PrdClausesQueue& get(uint32_t thread_id) const;

};

}

#endif
