#ifndef _DPS_DET_PARALLEL_SOLVER_H_
#define _DPS_DET_PARALLEL_SOLVER_H_

#include <vector> 
#include <thread>

// Include files should be specified relatively to avoid confusion with same named files.
#include "Sharer.h"
#include "Options.h"
#include "../solvers/AbstDetSeqSolver.h"

namespace DPS {

using std::string;
using std::vector;

class DetParallelSolver {
protected:
    Sharer*                     sharer;    
    uint32_t                    num_threads;    // number of threads
    vector<AbstDetSeqSolver*>   solvers;        // pointers to solver objects
    vector<pthread_t*>          threads;        // all threads of this process
    Instance                    input_formula;  // input formula
    Options                     options;        // options

    pthread_mutex_t             mfinished;      // mutex on which main process may wait for... As soon as one process finishes it release the mutex
    pthread_cond_t              cfinished;      // condition variable that says that a thread has finished
    
    double start_real_time;                     // start time to begin solving
    double used_mem_after_loading;              // used memory after loading input formula

    uint32_t num_print_stats;

    void   generateAllSolvers();

public:
    DetParallelSolver();
    ~DetParallelSolver();

    void setOptions(int argc, const char* const argv[]) { options.setOptions(argc, argv); }
    
    // TODO: implement interface to input formula
    // addClause(Clause& c)

    SATResult solve();
    SATResult getResult() const { return sharer ? sharer->getResult() : UNKNOWN; }

    bool      verbose() { return options.verbose(); }
    void      printStats();
    void      printResult();
};

}

#endif