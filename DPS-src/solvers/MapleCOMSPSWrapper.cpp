// Include files should be specified relatively to avoid confusion with same named files.
#include "MapleCOMSPSWrapper.h"

using namespace DPS;
using namespace MapleCOMSPS;

// Macros for MapleCOMSPS literal representation conversion
#define INT_LIT(lit) sign(lit) ? -(var(lit) + 1) : (var(lit) + 1)

MapleCOMSPSWrapper::MapleCOMSPSWrapper(int id, Sharer *sharer, Options& options) : 
    AbstDetSeqSolver(id, sharer, options)
,   exp_clause_lbd_lim(options.getMCLBDLim())    
,   exp_clause_len_lim(options.getMCLenLim())
{
    solver = new SimpSolver();
    if (!solver) throw std::runtime_error("could not allocate memory for MapleCOMSPSWrapper::SimpSolver");
    solver->wrapper = this;
    solver->random_seed += id;    
    if (id) solver->rand_pick_until_1st_conf = true;

    // diversification strategy from painless
    if (id % 2) 
      solver->VSIDS = false;
    else
      solver->VSIDS = true;
    if (id % 4 >= 2) 
      solver->pick_strict = false;
    else 
      solver->pick_strict = true;   
}

MapleCOMSPSWrapper::~MapleCOMSPSWrapper() {
    delete solver;
}

SATResult MapleCOMSPSWrapper::solve() {
    if (input_formula != nullptr) {
        loadFormula(*input_formula);
        input_formula = nullptr;
    }

    // lbool res = solver->solveLimited(s->getAssumptions(), s->getDoSimp(), s->getTurnOffSimp());
    vec<Lit> dummy;
    parchrono.start(RunningTime);
    lbool res = solver->solveLimited(dummy, options.getMCSimp(), true);
    parchrono.stop(RunningTime);
    SATResult result = UNKNOWN;
    if (res == l_True) 
        result = SAT;
    else if (res == l_False)
        result = UNSAT;
    sharer->IFinished(result, periods, thn);
    completeCurrPeriod();
    pthread_cond_signal(pcfinished);
    return result;
}

bool MapleCOMSPSWrapper::loadFormula(const Instance& clauses) {
    vec<Lit> c;
    for (auto clause : clauses) {
        c.clear();
        for (auto n : clause) {
            Var v = abs(n) - 1;
            while (v >= solver->nVars()) solver->newVar();
            c.push(n > 0 ? mkLit(v) : ~mkLit(v));            
        }
        if (!solver->addClause(c))
            return false;
    }
    return true;
}

uint64_t MapleCOMSPSWrapper::getNumConflicts()     { return (uint64_t)solver->conflicts; }
uint64_t MapleCOMSPSWrapper::getNumDecisions()     { return (uint64_t)solver->decisions; }
uint64_t MapleCOMSPSWrapper::getNumPropagations()  { return (uint64_t)solver->propagations; }
uint64_t MapleCOMSPSWrapper::getNumRestarts()      { return (uint64_t)solver->starts; }
uint64_t MapleCOMSPSWrapper::getNumRedundantClauses() { return (uint64_t)solver->nLearnts(); }

Model MapleCOMSPSWrapper::getModel() {
    Model model(solver->nVars());
    for (int i = 0; i < solver->nVars(); i++) {
        if (solver->model[i] != l_Undef) {
            bool val = solver->model[i] == l_True;
            model[i+1] = val;
        }
    }
    return model;
}

void MapleCOMSPSWrapper::exportClause(vec<Lit>& cls, uint32_t lbd) {
    lbd_dist.add(lbd);
    if (2 < lbd && 0 < exp_clause_len_lim && exp_clause_len_lim < (uint32_t)cls.size())  // a glue clause is exported even if the length is too long.
        return;
    if (0 < exp_clause_lbd_lim && exp_clause_lbd_lim < lbd)
        return;
    if (2 < lbd && getLBDUpperbound() < lbd)
        return;

    parchrono.start(ExchangingTime);
    // convert glucose's clause to DPS's clause
    exp_tmp.clear();
    for (int i=0; i < cls.size(); i++) 
        exp_tmp.push_back(INT_LIT(cls[i]));

    // export clause
    exp_clauses_buf.addClause(exp_tmp, exp_tmp.size());
    parchrono.stop(ExchangingTime);
}