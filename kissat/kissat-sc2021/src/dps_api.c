// APIs added by nabesima
#include "dps_api.h"
#include "internal.h"
#include "backtrack.h"
#include "error.h"
#include "require.h"
#include "import.h"
#include "inline.h"
#include "propsearch.h"
#include "literal.h"
#include "options.h"
#include "../../../DPS-src/solvers/DPS_C_API.h"
#include "../../../DPS-src/solvers/ThreadLocalVars.h"

void kissat_set_wrapper(kissat *solver, void *wrapper, int thn) {
    solver->wrapper = wrapper;
    solver->thn = thn;
}

void kissat_rand_pick_until_1st_conf(kissat *solver, int use) {
    solver->rand_pick_until_1st_conf = use;
}

unsigned int kissat_get_decision_level(kissat *solver) {
    return solver->level;
}

void kissat_forced_restart(kissat *solver) {
    kissat_backtrack_in_consistent_state (solver, 0);
}

int kissat_add_imported_unit_clause(kissat *solver, int elit) {
    if (solver->inconsistent)
        return -1;
    kissat_require_valid_external_internal (elit);
    unsigned ilit = kissat_import_literal (solver, elit);
    if (ilit == INVALID_LIT)
        return +1;
    if (ELIMINATED (IDX (ilit)))
        return +1;
    const value value = kissat_fixed(solver, ilit);
    if (value > 0)  // satisfied
        return +1;
    if (value < 0)  // falsified
        return -1;
    kissat_learned_unit (solver, ilit);
    (void)kissat_search_propagate (solver);
    return solver->inconsistent ? -1: 0;
}

int kissat_add_imported_clause(kissat *solver, const int *clause, unsigned size) {
    assert (EMPTY_STACK (solver->clause));
    if (solver->inconsistent)
        return -1;
    
    const int *p = clause;
    const int *const end = clause + size;
    for (; p != end; p++) {
        int elit = *p;
        kissat_require_valid_external_internal (elit);
        unsigned ilit = kissat_import_literal (solver, elit);
        if (ilit == INVALID_LIT)
            break;
        if (ELIMINATED (IDX (ilit)))
            break;  // can't add
        const value value = kissat_fixed(solver, ilit);
        if (value > 0)  // satisfied
            break;
        if (value < 0)  // falsified
            continue;

        PUSH_STACK (solver->clause, ilit);
    }
    if (p != end) {
        CLEAR_STACK (solver->clause);
        return +1;
    }

    size = SIZE_STACK (solver->clause);
    if (size == 0)  // empty clause
        return -1;
    if (size == 1) {
        kissat_learned_unit (solver, PEEK_STACK (solver->clause, 0));
        (void)kissat_search_propagate (solver);
    }
    else
        kissat_new_redundant_clause (solver, size - 1);

    CLEAR_STACK (solver->clause);
    
    return solver->inconsistent ? -1 : 0;
}

// statistics
uint64_t kissat_get_num_conflicts(kissat *solver)    { return solver->statistics.conflicts; }
uint64_t kissat_get_num_decisions(kissat *solver)    { return solver->statistics.decisions; }
uint64_t kissat_get_num_propagations(kissat *solver) { return solver->statistics.propagations; }
uint64_t kissat_get_num_restarts(kissat *solver)     { return solver->statistics.restarts; }
uint64_t kissat_get_num_redudants(kissat *solver)    { return solver->statistics.clauses_redundant; }
char     kissat_get_solver_state(kissat *solver)     { return solver->solver_state; }

bool dps_check_period(kissat *solver, const char *msg) {
  if (solver->should_be_terminated)
    return false;
  void *wrapper = solver->wrapper;
  if (!wrapper) 
    return true;
  num_mem_accesses += solver->dps_ticks;
  solver->dps_ticks = 0;
  bool res = DPS_checkPeriod(wrapper, msg);
  if (!res)
    solver->should_be_terminated = true;
  return res;
}