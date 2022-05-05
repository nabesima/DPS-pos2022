#ifndef _DPS_KISSAT_WRAPPER_H_
#define _DPS_KISSAT_WRAPPER_H_

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
extern "C" {
   #include "../../kissat/kissat-sc2021/src/dps_api.h"
}

namespace DPS {

class KissatWrapper : public AbstDetSeqSolver {
private:
   kissat*  solver;
   uint32_t num_vars;
   Clause   exp_tmp;
   double   exp_clause_lbd_lim;

public:   
   KissatWrapper(int id, Sharer *sharer, Options& options);
   ~KissatWrapper();

   SATResult solve();
   bool loadFormula(const Instance& clauses);
   void incExpClauseGen() { exp_clause_lbd_lim += 1.0 / exp_clause_lbd_lim; }
   void decExpClauseGen() { exp_clause_lbd_lim -= 1.0; exp_clause_lbd_lim = std::max(2.0, exp_clause_lbd_lim); }

   bool shouldBeExported(uint32_t lbd);
   void uncheckedExportClause(const int *clause, uint32_t len, uint32_t lbd);

   kissat* getKissatSolver() { return solver; }

   // statistics methods
   uint64_t getNumConflicts();
   uint64_t getNumPropagations();
   uint64_t getNumDecisions();
   uint64_t getNumRestarts();
   uint64_t getNumRedundantClauses();
   Model    getModel();
   uint32_t getExpLBDthreshold() { return std::min(exp_clause_lbd_lim, getLBDUpperbound()); }
   char     getSolverState();
};

}

extern "C" {
   // copied from kissat/src/handle.c
   void kissat_init_signal_handler (void (*handler) (int));
   void kissat_reset_signal_handler (void);

   void kissat_init_alarm (void (*handler) (void));
   void kissat_reset_alarm (void);

   const char *kissat_signal_name (int sig);
}

#endif