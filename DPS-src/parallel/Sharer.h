#ifndef _DPS_SHARER_H_
#define _DPS_SHARER_H_

#include <atomic>

#include "../period/PrdClausesQueue.h"
#include "../period/PrdClausesQueueMgr.h"

namespace DPS {

enum SATResult {
	SAT     = 10,
	UNSAT   = 20,
	UNKNOWN = 0
};

class AbstDetSeqSolver;

class Sharer {
    friend class DetParallelSolver;
    friend class AbstDetSeqSolver;

public:
    Sharer(uint32_t _num_threads, uint32_t _margin, uint64_t _mem_acc_lim);
    ~Sharer();

    uint32_t getNumThreads() const { return num_threads; }
    uint32_t getMargin()     const { return margin; }
    uint64_t getMemAccLim()  const { return mem_acc_lim; }
    void incNumLiveThreads();
    void decNumLiveThreads();
    int  getNumLiveSolvers();
    bool hasNoLiveThreads()  { return num_live_threads == 0; }

    void completeCurrPeriod(int thn, uint64_t prd_len);
    bool shouldBeTerminated(uint64_t prd);
    bool IFinished(SATResult status,uint64_t prd,int thn);

    PrdClausesQueue& get(int thread_id) const;
    
    int getWinner() {return winner_id; }
    SATResult getResult() { return final_result; }
    

protected:
    uint32_t num_threads;
    uint32_t margin;
    uint64_t mem_acc_lim;

    std::atomic<int> num_live_threads;
    std::atomic<bool> lanched;
    std::atomic<bool> sol_found;
    std::atomic<SATResult> final_result;
    uint64_t winner_period;
    int winner_id;
    
    PrdClausesQueueMgr * pcqm;
    
    pthread_mutex_t mutexJobFinished;
    //shared_mutex mutexJobFinished;
};

}

#endif