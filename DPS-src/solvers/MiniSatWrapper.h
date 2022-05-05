#ifndef _DPS_MINISAT_WRAPPER_H_
#define _DPS_MINISAT_WRAPPER_H_

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
#include "../../minisat/minisat-2.2.0/simp/SimpSolver.h"

namespace DPS {

class MiniSatWrapper : public AbstDetSeqSolver {
private:
   Minisat::SimpSolver* solver;
   Clause               exp_tmp;
   int                  exp_clause_len_lim;

public:   
   MiniSatWrapper(int id, Sharer *sharer, Options& options);
   ~MiniSatWrapper();

   SATResult solve();
   bool loadFormula(const Instance& clauses);

   void exportClause(Minisat::vec<Minisat::Lit>& cls);

   // statistics methods
   uint64_t getNumConflicts();
   uint64_t getNumPropagations();
   uint64_t getNumDecisions();
   uint64_t getNumRestarts();
   uint64_t getNumRedundantClauses();
   Model    getModel();
};

}

#endif