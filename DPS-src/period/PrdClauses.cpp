#include <assert.h>

#include "PrdClauses.h"

using namespace DPS;

PrdClauses::PrdClauses(int thread_id, uint64_t period) :
        thn(thread_id),
        prd(period),
        prd_len(0),
        num_literals(0),
        num_exported_threads(0),
        completed(false)
{
    pthread_mutex_init(&lock_num_exported_threads, NULL);
    pthread_mutex_init(&lock_completed, NULL);
    pthread_cond_init(&is_completed, NULL);
}

bool PrdClauses::addClause(const Clause& c) {
    assert(!completed);
    clauses.push_back(c);
    num_literals += c.size();
    return true;
}

// When the period of the thread is finished (it means that the addition of clauses is completed),
// then this method is called by the thread. This method notifies waiting threads to be completed.
void PrdClauses::completeAddition(uint64_t _prd_len) {
    assert(completed == false);
    pthread_mutex_lock(&lock_completed);
    completed = true;
    prd_len = _prd_len;
    // Send signals to threads which are waiting to be completed.
    pthread_cond_broadcast(&is_completed);
    pthread_mutex_unlock(&lock_completed);
}

// Wait the addition of clauses to be completed.
void PrdClauses::waitAdditionCompleted(void) {
    pthread_mutex_lock(&lock_completed);
    if (!completed)
        pthread_cond_wait(&is_completed, &lock_completed);
    pthread_mutex_unlock(&lock_completed);
}

bool PrdClauses::isAdditionCompleted(void) {
    pthread_mutex_lock(&lock_completed);
    int ret = completed;
    pthread_mutex_unlock(&lock_completed);
    return ret;
}

// When exporting to the specified thread is finished, then this method is called.
void PrdClauses::completeExportation(int thread_id) {
    pthread_mutex_lock(&lock_num_exported_threads);
    num_exported_threads++;
    pthread_mutex_unlock(&lock_num_exported_threads);
}

int PrdClauses::getNumExportedThreads(void) {
    pthread_mutex_lock(&lock_num_exported_threads);
    int num = num_exported_threads;
    pthread_mutex_unlock(&lock_num_exported_threads);
    return num;
}
