#ifndef _DPS_PRD_CLAUSES_H_
#define _DPS_PRD_CLAUSES_H_

#include <vector>
#include <pthread.h>
#include <cstdint>

#include "../sat/Clause.h"

namespace DPS {

// A set of clauses acquired with a certain period of a thread
class PrdClauses {
private:
    int thn;                            // thread number
    uint64_t prd;                       // period number
    uint64_t prd_len;                   // period length for adaptive strategy
    size_t   num_literals;              // the number of literals
    std::vector<Clause> clauses;        // a set of literals

    int num_exported_threads;           // the number of threads to which these clauses are exported.
    pthread_mutex_t lock_num_exported_threads;    // mutex on the variable "num_exported_threads"

    bool completed;                     // whether the addition of clauses from the thread is finished
    pthread_mutex_t lock_completed;     // mutex on the variable "completed"
    pthread_cond_t  is_completed;       // condition variable that says that this set of clauses is completed

public:
    PrdClauses(int thread_id, uint64_t period);

    bool addClause(const Clause& c);

    // When the period of the thread is finished (it means that the addition of clauses is completed),
    // then this method is called by the thread. This method notifies waiting threads to be completed.
    void completeAddition(uint64_t prd_len);

    // Wait the addition of clauses to be completed.
    void waitAdditionCompleted(void);

    // Methods for exportation
    int size(void) const { return clauses.size(); };
    const Clause& operator [] (int index) const { return clauses[index]; }

    // When exporting to the specified thread is finished, then this method is called.
    void completeExportation(int thread_id);
    int getNumExportedThreads(void);

    // Misc
    uint64_t  period(void)          const { return prd; }
    uint64_t  getPrdLenCand(void)   const { return prd_len; }
    uint32_t  getNumClauses(void)   const { return clauses.size(); }
    uint32_t  getNumLiterals(void)  const { return num_literals; }
};

}

#endif
