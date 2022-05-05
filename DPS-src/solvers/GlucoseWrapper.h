#ifndef _DPS_GLUCOSE_WRAPPER_H_
#define _DPS_GLUCOSE_WRAPPER_H_

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
#include "../../glucose/glucose-3.0/simp/SimpSolver.h"

namespace DPS {

class GlucoseWrapper : public AbstDetSeqSolver {
private:
   Glucose::SimpSolver* solver;
   Clause               exp_tmp;
   uint32_t             exp_clause_lbd_lim;
   uint32_t             exp_clause_len_lim;

public:   
   GlucoseWrapper(int id, Sharer *sharer, Options& options);
   ~GlucoseWrapper();

   SATResult solve();
   bool loadFormula(const Instance& clauses);
   void incExpClauseGen() { exp_clause_lbd_lim++; }
   void decExpClauseGen() { if (exp_clause_lbd_lim > 2) exp_clause_lbd_lim--; }

   void exportClause(Glucose::vec<Glucose::Lit>& cls, uint32_t lbd);

   // statistics methods
   uint64_t getNumConflicts();
   uint64_t getNumPropagations();
   uint64_t getNumDecisions();
   uint64_t getNumRestarts();
   uint64_t getNumRedundantClauses();
   Model    getModel();
   uint32_t getExpLBDthreshold() { return exp_clause_lbd_lim; }
};

}

#endif