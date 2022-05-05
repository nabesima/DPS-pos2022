#ifndef _DPS_SOLVER_FACTORY_H_
#define _DPS_SOLVER_FACTORY_H_

#include <vector>
#include <string>

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
#include "../parallel/Options.h"

namespace DPS{

class SolverFactory {
public:
    // Instantiate and return SAT solvers.
    static std::vector<AbstDetSeqSolver*> createSATSolvers(int num_solvers, Sharer *sharer, Options& options);

    // Instantiate and return a SAT solver.
    static AbstDetSeqSolver* createSATSolver(std::string name, Sharer *sharer, Options& options);
};

}

#endif