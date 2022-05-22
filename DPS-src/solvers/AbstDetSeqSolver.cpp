#define ABSTRACT_DET_SEQ_SOLVER_CPP

#include <inttypes.h>

// Include files should be specified relatively to avoid confusion with same named files.
#include "AbstDetSeqSolver.h"
#include "../utils/System.h"

using namespace DPS;

AbstDetSeqSolver::AbstDetSeqSolver(int id, Sharer *_sharer, Options& _options
    // uint32_t _margin, uint64_t _mem_acc_lim, 
    // uint32_t _fapp_clauses, uint32_t _fapp_periods, uint32_t _exp_lits_lim, double _exp_lits_margin
) :
    thn(id)
,   sharer(_sharer)
,   options(_options)
,   margin(options.getMargin())
,   periods(0)
,   mem_acc_lim(options.getMemAccLim())
,   prd_clauses_queue(sharer->get(thn))
,   fapp_clauses(options.getFAppClauses())
,   fapp_periods(options.getFAppPeriods())
,   last_fapp_period(0)
,   sum_mem_accs(0)
,   ema_mem_accs(options.getMemAccLim())
,   ema_prev_confs(0)
,   next_mem_acc_lim(options.getMemAccLim())
,   exp_lits_lim(options.getExpLitsLim())
,   exp_lits_margin(options.getExpLitsMargin())
,   prev_exp_lits(0)
,   prev_exp_confs(0)
,   start_real_time(realTime())
,   real_time_lim(options.getRealTimeLim())
,   mem_use_lim(options.getMemUseLim())
,   num_imported_clauses(0)
,   num_exported_clauses(0)
,   num_forced_applications(0)
,   input_formula(nullptr)
,   pmfinished(nullptr)
,   pcfinished(nullptr)
{
    lbd_dist.add_equal_spacing(20); // 0%, 5%, 10%, ...
}

bool AbstDetSeqSolver::checkPeriod(const char *msg) {
    if (canMoveToNextPeriod()) {
        if (thn == 0 && options.verbose() > 2 &&  num_mem_accesses > mem_acc_lim * 1.1)
            printf("c T%02d: EXCEED MEM ACC LIM %" PRIu64 "/%" PRIu64 " = %.1f (%s)\n", thn, num_mem_accesses, mem_acc_lim, (double)num_mem_accesses / mem_acc_lim,  msg ? msg : "");

        parchrono.start(ExchangingTime);
        exportSelectedClauses();

        parchrono.toggle(ExchangingTime, PeriodUpdateTime);
        uint64_t prd_len = getNewPeriodLength();
        //printf("c T%02d: n = %" PRIu64 "\n", thn, prd_len);

        sharer->completeCurrPeriod(thn, prd_len);

        parchrono.toggle(PeriodUpdateTime, ExchangingTime);
        importClauses();
        
        parchrono.toggle(ExchangingTime, PeriodUpdateTime);
        moveToNextPeriod();

        parchrono.stop(PeriodUpdateTime);

        if (shouldBeTerminated())
            return false;        
    }
    return true;
}

void AbstDetSeqSolver::exportSelectedClauses() {
    uint32_t before_lits = exp_clauses_buf.getNumLiterals();  // DEBUG
    uint32_t exp_clauses = exp_clauses_buf.exportTo(prd_clauses_queue.last(), exp_lits_lim);    
    uint32_t after_lits = exp_clauses_buf.getNumLiterals();   // DEBUG    
    num_exported_clauses += exp_clauses;

    if (exp_lits_lim == 0) return;

    uint64_t curr_exp_confs = getNumConflicts();
    bool no_conf = curr_exp_confs == prev_exp_confs;
    prev_exp_confs = curr_exp_confs;
    if (!no_conf) {
        // HordeSAT-style exported clauses control strategy
        uint32_t curr_exp_lits = exp_clauses_buf.getNumLiterals();
        int32_t  diff_exp_lits = curr_exp_lits - prev_exp_lits;
        prev_exp_lits = curr_exp_lits;

        if (curr_exp_lits < exp_lits_lim * exp_lits_margin && diff_exp_lits <= 0)
            incExpClauseGen();
        else if (curr_exp_lits > exp_lits_lim * exp_lits_margin && diff_exp_lits > 0)
            decExpClauseGen();
    }

    uint32_t lbd = getExpLBDthreshold();
    if (thn == 0 && options.verbose() > 2) 
        printf("c T%02d: EXPORT %5d -> %5d (%5d lits, %4d clas), lbd = %2d\n", thn, before_lits, after_lits, before_lits - after_lits, exp_clauses, lbd);
}

uint64_t AbstDetSeqSolver::getNewPeriodLength() {
    uint32_t base_confs = options.getAdptPrd();
    if (base_confs == 0) return mem_acc_lim;

    // use adaptive period-length strategy
    sum_mem_accs += mem_acc_lim;
    uint64_t curr_confs = getNumConflicts();    
    if (curr_confs > ema_prev_confs + base_confs) {
        uint64_t confs = curr_confs - ema_prev_confs;

        double alpha   = options.getAdptPrdSmth();
        double new_val = ((double)sum_mem_accs / (double)confs) * base_confs; // # of memory accesses to generate 'base_confs' conflicts
        ema_mem_accs = alpha * new_val + (1.0 - alpha) * ema_mem_accs;
        
        sum_mem_accs = 0;
        ema_prev_confs = curr_confs;
    }

    return ema_mem_accs;
}

void AbstDetSeqSolver::moveToNextPeriod() {
    assert(num_mem_accesses >= mem_acc_lim);
    num_mem_accesses -= mem_acc_lim;
    periods++;
    mem_acc_lim = next_mem_acc_lim;
}

bool AbstDetSeqSolver::importClauses() {
    if (periods < margin) return false;
    
    uint64_t sum_prd_len_cand = 0;    
    for (uint32_t i=1; i < sharer->num_threads; i++) {      // i=0 denote the current thread
        uint32_t target = (thn + i) % sharer->num_threads;  // target thread number from which clauses are imported
        PrdClausesQueue& queue = sharer->get(target);
        PrdClauses* p = NULL;
        while ((p = queue.get(thn, periods - margin)) != NULL) {
            PrdClauses& prdClauses = *p;

            if (options.getNonDetMode()) {
                if (!prdClauses.isAdditionCompleted())
                    break;
            }
            else {
                parchrono.start(WaitingTime);
                prdClauses.waitAdditionCompleted();
                parchrono.stop(WaitingTime);
            }

            for (int j=0; j < prdClauses.size(); j++) {
                const Clause& c = prdClauses[j];
                if (c.size() > 1)
                    imported_clauses.push_back(c);
                else
                    imported_unit_clauses.push_back(c);
            }
            sum_prd_len_cand += prdClauses.getPrdLenCand();
            queue.completeExportation(thn, prdClauses);
        }
    }

    next_mem_acc_lim = mem_acc_lim;
    if (options.getAdptPrd()) {
        PrdClausesQueue& queue = sharer->get(thn);
        PrdClauses* p = queue.get(periods - margin);
        assert(p != nullptr);
        PrdClauses& prdClauses = *p;
        sum_prd_len_cand += prdClauses.getPrdLenCand();
        // PrdClauses is used as a data folder
        queue.completeExportation(thn, prdClauses);
        //printf("c T%02d,P%" PRIu64 ": next_prd_len = %" PRIu64 "\n", thn, periods, sum_prd_len_cand / sharer->num_threads);
        next_mem_acc_lim = sum_prd_len_cand / sharer->num_threads;
        uint64_t lb = options.getAdptPrdLB();
        uint64_t ub = options.getAdptPrdUB();
        if (lb != 0 && next_mem_acc_lim < lb) next_mem_acc_lim = lb;
        if (ub != 0 && ub < next_mem_acc_lim) next_mem_acc_lim = ub;
    }

    return true;
}

bool AbstDetSeqSolver::shouldApplyImportedClauses() {
    // If unit clauses exist, then it should be applied immediately
    if (imported_unit_clauses.size() > 0
        && periods >= last_fapp_period + fapp_periods) {
        last_fapp_period = periods;
        return true;
    }
    // If there are many imported clauses, then it should be applied
    if (fapp_clauses > 0 
        && imported_clauses.size() / sharer->getNumThreads() >= fapp_clauses 
        && periods >= last_fapp_period + fapp_periods) {
        last_fapp_period = periods;
        return true;
    }
    return false;
}

bool AbstDetSeqSolver::shouldBeTerminated() {
    return sharer->shouldBeTerminated(periods) 
        || (mem_use_lim > 0 && usedMemory() > mem_use_lim)
        || (real_time_lim > 0 && realTime() > start_real_time + real_time_lim);    
}
