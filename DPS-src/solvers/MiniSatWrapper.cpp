// Include files should be specified relatively to avoid confusion with same named files.
#include "MiniSatWrapper.h"

using namespace DPS;
using namespace Minisat;

// Macros for minisat literal representation conversion
#define INT_LIT(lit) sign(lit) ? -(var(lit) + 1) : (var(lit) + 1)

MiniSatWrapper::MiniSatWrapper(int id, Sharer *sharer, Options& options) : 
    AbstDetSeqSolver(id, sharer, options)
,   exp_clause_len_lim(options.getMSLenLim())    
{
    solver = new SimpSolver();
    if (!solver) throw std::runtime_error("could not allocate memory for Minisat::SimpSolver");
    solver->wrapper = this;
    solver->random_seed += id;    
    if (id) solver->rand_pick_first_conflict = true;
}

MiniSatWrapper::~MiniSatWrapper() {
    delete solver;
}

SATResult MiniSatWrapper::solve() {
    if (input_formula != nullptr) {
        loadFormula(*input_formula);
        input_formula = nullptr;
    }

    vec<Lit> dummy;
    parchrono.start(RunningTime);
    lbool res = solver->solveLimited(dummy, options.getMSSimp(), true);
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

bool MiniSatWrapper::loadFormula(const Instance& clauses) {
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

uint64_t MiniSatWrapper::getNumConflicts()    { return (uint64_t)solver->conflicts; }
uint64_t MiniSatWrapper::getNumDecisions()    { return (uint64_t)solver->decisions; }
uint64_t MiniSatWrapper::getNumPropagations() { return (uint64_t)solver->propagations; }
uint64_t MiniSatWrapper::getNumRestarts()     { return (uint64_t)solver->starts; }
uint64_t MiniSatWrapper::getNumRedundantClauses() { return (uint64_t)solver->nLearnts(); }

Model MiniSatWrapper::getModel() {
    Model model(solver->nVars());
    for (int i = 0; i < solver->nVars(); i++) {
        if (solver->model[i] != l_Undef) {
            bool val = solver->model[i] == l_True;
            model[i+1] = val;
        }
    }
   return model;
}

void MiniSatWrapper::exportClause(vec<Lit>& cls) {
	if (0 < exp_clause_len_lim && exp_clause_len_lim < cls.size())
		return;

    parchrono.start(ExchangingTime);
    // convert minisat's clause to DPS's clause
    exp_tmp.clear();
    for (int i=0; i < cls.size(); i++) 
        exp_tmp.push_back(INT_LIT(cls[i]));

    // export clause
    exp_clauses_buf.addClause(exp_tmp, exp_tmp.size());
    parchrono.stop(ExchangingTime);
}