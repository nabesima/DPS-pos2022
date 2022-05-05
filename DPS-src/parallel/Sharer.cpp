#include <iostream>
#include <cassert>
#include <stdexcept>

#include "Sharer.h"
#include "../solvers/AbstDetSeqSolver.h"

using namespace DPS;
using namespace std;

Sharer::Sharer(uint32_t _num_threads, uint32_t _margin, uint64_t _mem_acc_lim):
    num_threads(_num_threads)
,   margin(_margin)    
,   mem_acc_lim(_mem_acc_lim)
,   num_live_threads(0)
,   lanched(false)
,   sol_found(false)
,   final_result(UNKNOWN)
,   winner_period(0)
,   winner_id(-1)
{
    // assert(queues.size() == 0);
    pcqm = new PrdClausesQueueMgr(num_threads);
    if (!pcqm) throw std::runtime_error("could not allocate memory for PrdClausesQueueMgr");
    pthread_mutex_init(&mutexJobFinished, NULL); // This is the shared companion lock
}

Sharer::~Sharer() {}

void Sharer::incNumLiveThreads() {
    assert(num_live_threads >= 0);
    // assert(nbLiveSolvers < nbSolvers);
    num_live_threads++;
    lanched = true;
}

void Sharer::decNumLiveThreads() {
    assert(num_live_threads >= 1);
    num_live_threads--;
}

int Sharer::getNumLiveSolvers() {
    if (lanched == true)
        return num_live_threads;
    return -1;
}

void Sharer::completeCurrPeriod(int thn, uint64_t prd_len) {
    pcqm->get(thn).completeAddtion(prd_len);
}

bool Sharer::shouldBeTerminated(uint64_t prd) {
    bool ret = false;
    pthread_mutex_lock(&mutexJobFinished);  // TODO: read only mutex is available here.
    // modified by nabesima
    //shared_lock<shared_mutex> lock(mutexJobFinished);       // readers can access simultaneously
    ret = sol_found && (final_result == UNKNOWN || winner_period + margin < prd);
    pthread_mutex_unlock(&mutexJobFinished);
    return ret;
}

bool Sharer::IFinished(SATResult status, uint64_t prd, int thn) {
    bool found_better_one = false;

    pthread_mutex_lock(&mutexJobFinished);
    //lock_guard<shared_mutex> lock(mutexJobFinished);

    assert(status == UNKNOWN || final_result == UNKNOWN || final_result == status);

    // modified by nabesima
    if (!sol_found
            || (status != UNKNOWN && prd <  winner_period)
            || (status != UNKNOWN && prd == winner_period && thn < winner_id)
       ) {
        found_better_one = true;
        sol_found        = true;
        winner_id        = thn;
        winner_period    = prd;   
        final_result     = status;
    }

    pthread_mutex_unlock(&mutexJobFinished);
    return found_better_one;
}

PrdClausesQueue& Sharer::get(int thread_id) const {
    assert(0 <= thread_id);
    return pcqm->get(thread_id);
}
