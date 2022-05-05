#include <stdexcept>

#include "PrdClausesQueue.h"

using namespace DPS;

PrdClausesQueue::PrdClausesQueue(int _thn, int _num_threads) :
    thn(_thn)
,   num_threads(_num_threads)
,   next_period(std::vector<uint64_t>(_num_threads))
{
    pthread_rwlock_init(&rwlock, NULL);

    // Add an empty set of clauses to which clauses acquired at period 0 are stored.
    PrdClauses *pcs = new PrdClauses(thn, 0);
    if (!pcs) throw std::runtime_error("could not allocate memory for PrdClauses");
    queue.push_back(pcs);
}

PrdClausesQueue::~PrdClausesQueue()
{
    queue.clear();
}

// When the current period of the thread is finished, then this method is called by the thread.
// This method notifies waiting threads to be completed.
void PrdClausesQueue::completeAddtion(uint64_t prd_len)
{
    assert(queue.size() > 0);
    PrdClauses& last = *queue.back();

    pthread_rwlock_wrlock(&rwlock);

    // Add an empty set of clauses to which clauses acquired at the next period are stored.
    PrdClauses *pcs = new PrdClauses(thn, last.period() + 1);
    if (!pcs) throw std::runtime_error("could not allocate memory for PrdClauses");
    queue.push_back(pcs);

    // Complete and notify it to all waiting threads
    last.completeAddition(prd_len);

    // Remove a set of clauses that were sent to other threads
    while (queue.size() > 1) {
        PrdClauses* head = queue.front();
        if (head->getNumExportedThreads() != num_threads)
            break;
        queue.erase(queue.begin());
        // delete head;
    }

    pthread_rwlock_unlock(&rwlock);
}

// When exporting to the specified thread is finished, then this method is called.
void PrdClausesQueue::completeExportation(int thn, PrdClauses& prdClauses)
{
    assert(next_period[thn] == prdClauses.period());
    next_period[thn]++;
    prdClauses.completeExportation(thn);
}


// Get a set of clauses which are generated at the specified period.
PrdClauses* PrdClausesQueue::get(int thread, uint64_t period) {
    uint64_t p = next_period[thread];
    if (period < p) return NULL;    // 'period' is already exported to 'thn'

    pthread_rwlock_rdlock(&rwlock);
    assert(queue.size() > 0);

    assert(queue.front()->period() <= p);

    size_t index = p - queue.front()->period();
    assert(0 <= index);
    assert(index < queue.size());
    PrdClauses *prdClauses = queue[index];
    
    assert(prdClauses->period() == p);
    pthread_rwlock_unlock(&rwlock);

    return prdClauses;
}

// Get a set of clauses which are generated at the specified period.
PrdClauses* PrdClausesQueue::get(uint64_t period) {
    assert(queue.front()->period() <= period);
    size_t index = period - queue.front()->period();
    assert(0 <= index);
    assert(index < queue.size());
    PrdClauses *prdClauses = queue[index];
    assert(prdClauses->period() == period);
    return prdClauses;
}




