// APIs added by nabesima
#ifndef _KISSAT_DPS_API_H_
#define _KISSAT_DPS_API_H_

#include <stdint.h>
#include <stdbool.h>
#include "kissat.h"

void kissat_set_wrapper(kissat *solver, void *wrapper, int thn);
int  kissat_set_option(kissat *solver, const char *name, int new_value); // defined in internal.c
void kissat_rand_pick_until_1st_conf(kissat *solver, int use);
unsigned int kissat_get_decision_level(kissat *solver);
void kissat_forced_restart(kissat *solver);
int kissat_add_imported_unit_clause(kissat *solver, int elit);
int kissat_add_imported_clause(kissat *solver, const int *clause, unsigned size);

// statistics
uint64_t kissat_get_num_conflicts(kissat *solver);
uint64_t kissat_get_num_decisions(kissat *solver);
uint64_t kissat_get_num_propagations(kissat *solver);
uint64_t kissat_get_num_restarts(kissat *solver);
uint64_t kissat_get_num_redudants(kissat *solver);
char     kissat_get_solver_state(kissat *solver);

bool dps_check_period(kissat *solver, const char *msg);

#endif