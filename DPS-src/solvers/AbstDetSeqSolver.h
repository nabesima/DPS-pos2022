#ifndef _DPS_ABST_DET_SEQ_SOLVER_H_
#define _DPS_ABST_DET_SEQ_SOLVER_H_

#include <zlib.h>
#include <iostream>

// Include files should be specified relatively to avoid confusion with same named files.
#include "ThreadLocalVars.h"
#include "../parallel/Sharer.h"
#include "../parallel/Options.h"
#include "../period/ClauseBuffer.h"
#include "../sat/Instance.h"
#include "../sat/Model.h"
#include "../utils/p2.h"
#include "../utils/Chronometer.h"

namespace DPS {

class AbstDetSeqSolver {
    friend class DetParallelSolver;

protected:
    uint32_t            thn;
    Sharer*             sharer;    
    Options&            options;
    uint32_t            margin;
    uint64_t            periods;
    uint64_t            mem_acc_lim;
    PrdClausesQueue&    prd_clauses_queue;
    ClauseBuffer        exp_clauses_buf;
    std::vector<Clause> imported_clauses;
    std::vector<Clause> imported_unit_clauses;
    uint32_t            fapp_clauses;
    uint32_t            fapp_periods;    
    uint64_t            last_fapp_period;
    uint64_t            sum_mem_accs;
    uint64_t            ema_mem_accs;
    uint64_t            ema_prev_confs;
    uint64_t            next_mem_acc_lim;
    uint32_t            exp_lits_lim;
    double              exp_lits_margin;
    uint32_t            prev_exp_lits;
    uint64_t            prev_exp_confs;
    p2_t                lbd_dist;
    double              start_real_time;
    double              real_time_lim;
    double              mem_use_lim;
    uint64_t            num_imported_clauses;
    uint64_t            num_exported_clauses;
    uint64_t            num_forced_applications;
    
    // input formula that is shared with each solver    
    Instance const *input_formula;
    // mutex on which main process may wait for... As soon as one process finishes it release the mutex
    pthread_mutex_t *pmfinished; 
    // condition variable that says that a thread as finished
    pthread_cond_t *pcfinished; 

    bool     canMoveToNextPeriod() const { return num_mem_accesses >= mem_acc_lim; };
    void     exportSelectedClauses();
    uint64_t getNewPeriodLength(); 
    void     moveToNextPeriod();
    bool     importClauses();
    void     completeCurrPeriod() { return sharer->completeCurrPeriod(thn, getNewPeriodLength()); };

    Chronometer parchrono;    // chronometer for parallel proccessing

public:
    AbstDetSeqSolver(int id, Sharer *_sharer, Options& options);

    virtual ~AbstDetSeqSolver() {}
    
    bool     checkPeriod(const char *msg = nullptr);
    bool     shouldApplyImportedClauses();
    bool     shouldBeTerminated();
      
    // Main methods
    virtual SATResult solve() = 0;
    virtual bool loadFormula(const Instance& clauses) = 0;
    // Sharing strategy
    virtual void incExpClauseGen() {}
    virtual void decExpClauseGen() {}

    void setRealTimeLim(double time)             { real_time_lim = time; }
    void setMemUseLim(double mem)                { mem_use_lim = mem; }
    void setInputFormula(Instance const *p)      { input_formula = p; }

    int                     getThreadID()   const    { return thn; }
    Sharer*                 getSharer()     const    { return sharer; }
    uint64_t                getCurrPeriod() const    { return periods; }
    uint64_t                getMemAccLim()  const    { return mem_acc_lim; }
    std::vector<Clause>&    getImportedClauses()     { return imported_clauses; }
    std::vector<Clause>&    getImportedUnitClauses() { return imported_unit_clauses; }
    Chronometer&            getChronometer()         { return parchrono; }
    
    // statistics of base solver
    virtual uint64_t        getNumConflicts() = 0;
    virtual uint64_t        getNumDecisions() = 0;
    virtual uint64_t        getNumPropagations() = 0;
    virtual uint64_t        getNumRestarts() = 0;
    virtual uint64_t        getNumRedundantClauses() = 0;
    virtual Model           getModel() = 0;
    virtual uint32_t        getExpLBDthreshold() { return 0; }
    virtual char            getSolverState() { return ' '; }

    uint64_t getNumImportedClauses()    const    { return num_imported_clauses; }
    void     incNumImportedClauses()             { num_imported_clauses++;      }
    uint64_t getNumExportedClauses()    const    { return num_exported_clauses; }
    uint64_t getNumForcedApplications() const    { return num_forced_applications; }
    void     incNumForcedApplications()          { num_forced_applications++; }

    double   getLBDQuantile(double q)            { return lbd_dist.result(q); }
    double   getLBDUpperbound()                  { return getNumConflicts() < 1000 || options.getExpLBDQLim() == 1.0 ? UINT32_MAX : getLBDQuantile(options.getExpLBDQLim()); }
};

}

#endif