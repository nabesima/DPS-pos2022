#ifndef _DPS_PRD_CLAUSES_QUEUE_H
#define _DPS_PRD_CLAUSES_QUEUE_H

#include <cassert>
#include "PrdClauses.h"

namespace DPS {

// A set of clauses acquired at a certain thread
class PrdClausesQueue {
private:
    int thn;                                // thread number
    int num_threads;                        // the number of threads
    std::vector<uint64_t>     next_period;  // the next period for exporting to the specified thread
    std::vector<PrdClauses *> queue;        // a list of sets of clauses.
    pthread_rwlock_t          rwlock;       // read/write lock of this object.
    
public:
    PrdClausesQueue(int thread_id, int nb_threads);
    ~PrdClausesQueue();

    // When the current period of the thread is finished, then this method is called by the thread.
    // This method notifies waiting threads to be completed.
    void completeAddtion(uint64_t prd_len);

    // When exporting to the specified thread is finished, then this method is called.
    void completeExportation(int thread_id, PrdClauses& prdClauses);

    // Get a set of clauses which are generated at the specified period.
    PrdClauses* get(int thread, uint64_t period);

    // Get a set of own clauses which are generated at the specified period.
    PrdClauses* get(uint64_t period);

    // Return the last set of clauses
    PrdClauses& last() { assert(queue.size() > 0); return *queue[queue.size() - 1]; }
    
};

}

#endif
