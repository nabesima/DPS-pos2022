// Include files should be specified relatively to avoid confusion with same named files.
#include "GlucoseWrapper.h"

using namespace DPS;
using namespace Glucose;

// Macros for glucose literal representation conversion
#define INT_LIT(lit) sign(lit) ? -(var(lit) + 1) : (var(lit) + 1)

GlucoseWrapper::GlucoseWrapper(int id, Sharer *sharer, Options& options) : 
    AbstDetSeqSolver(id, sharer, options)
,   exp_clause_lbd_lim(options.getGLLBDLim())    
,   exp_clause_len_lim(options.getGLLenLim())    
{
    solver = new SimpSolver();
    if (!solver) throw std::runtime_error("could not allocate memory for Glucose::SimpSolver");
    solver->wrapper = this;
    solver->random_seed += id;    
    if (id) solver->rand_pick_first_conflict = true;
}

GlucoseWrapper::~GlucoseWrapper() {
    delete solver;
}

SATResult GlucoseWrapper::solve() {
    if (input_formula != nullptr) {
        loadFormula(*input_formula);
        input_formula = nullptr;
    }

    vec<Lit> dummy;
    parchrono.start(RunningTime);
    lbool res = solver->solveLimited(dummy, options.getGLSimp(), true);
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

bool GlucoseWrapper::loadFormula(const Instance& clauses) {
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

uint64_t GlucoseWrapper::getNumConflicts()     { return (uint64_t)solver->conflicts; }
uint64_t GlucoseWrapper::getNumDecisions()     { return (uint64_t)solver->decisions; }
uint64_t GlucoseWrapper::getNumPropagations()  { return (uint64_t)solver->propagations; }
uint64_t GlucoseWrapper::getNumRestarts()      { return (uint64_t)solver->starts; }
uint64_t GlucoseWrapper::getNumRedundantClauses() { return (uint64_t)solver->nLearnts(); }

Model GlucoseWrapper::getModel() {
    Model model(solver->nVars());
    for (int i = 0; i < solver->nVars(); i++) {
        if (solver->model[i] != l_Undef) {
            bool val = solver->model[i] == l_True;
            model[i+1] = val;
        }
    }
    return model;
}

void GlucoseWrapper::exportClause(vec<Lit>& cls, uint32_t lbd) {
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