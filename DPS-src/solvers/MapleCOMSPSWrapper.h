#ifndef _DPS_MAPLE_COMSPS_WRAPPER_H_
#define _DPS_MAPLE_COMSPS_WRAPPER_H_

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
#include "../../mcomsps/MapleCOMSPS_LRB_VSIDS_2_no_drup/MapleCOMSPS_LRB_VSIDS_2/simp/SimpSolver.h"

namespace DPS {

class MapleCOMSPSWrapper : public AbstDetSeqSolver {
private:
   MapleCOMSPS::SimpSolver* solver;
   Clause               exp_tmp;
   double               exp_clause_lbd_lim;
   uint32_t             exp_clause_len_lim;

public:   
   MapleCOMSPSWrapper(int id, Sharer *sharer, Options& options);
   ~MapleCOMSPSWrapper();

   SATResult solve();
   bool loadFormula(const Instance& clauses);
   void incExpClauseGen() { exp_clause_lbd_lim += 1.0 / exp_clause_lbd_lim; }
   //void decExpClauseGen() { exp_clause_lbd_lim -= 1.0 / exp_clause_lbd_lim; exp_clause_lbd_lim = std::max(2.0, exp_clause_lbd_lim); }
   void decExpClauseGen() { exp_clause_lbd_lim -= 1.0; exp_clause_lbd_lim = std::max(2.0, exp_clause_lbd_lim); }

   void exportClause(MapleCOMSPS::vec<MapleCOMSPS::Lit>& cls, uint32_t lbd);

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