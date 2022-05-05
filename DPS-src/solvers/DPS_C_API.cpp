#include "DPS_C_API.h"
#include "AbstDetSeqSolver.h"
#include "KissatWrapper.h"

namespace DPS {

int DPS_checkPeriod(void *wrapper, const char *msg) {
    AbstDetSeqSolver *solver = reinterpret_cast<AbstDetSeqSolver *>(wrapper);
    return solver->checkPeriod(msg);
}

int DPS_shouldBeTerminated(void *wrapper) {
    AbstDetSeqSolver *solver = reinterpret_cast<AbstDetSeqSolver *>(wrapper);
    return solver->shouldBeTerminated();
}

int DPS_shouldApplyImportedClauses(void *wrapper) {
    AbstDetSeqSolver *solver = reinterpret_cast<AbstDetSeqSolver *>(wrapper);
    return solver->shouldApplyImportedClauses();
}

// Kissat interfaces

bool DPS_kissat_shouldBeExported(void *wrapper, unsigned int lbd) {
    KissatWrapper *solver = reinterpret_cast<KissatWrapper *>(wrapper);
    return solver->shouldBeExported(lbd);
}

void DPS_kissat_uncheckedExportClause(void *wrapper, const int* clause, unsigned int size, unsigned int lbd) {
    KissatWrapper *solver = reinterpret_cast<KissatWrapper *>(wrapper);
    solver->uncheckedExportClause(clause, size, lbd);
}

int DPS_kissat_applyImportedClauses(void *wp) {
    KissatWrapper *wrapper = reinterpret_cast<KissatWrapper *>(wp);
    kissat *solver = wrapper->getKissatSolver();
    if (kissat_get_decision_level(solver) > 0) { 
        wrapper->incNumForcedApplications();
        kissat_forced_restart(solver);
    }
    
    wrapper->getChronometer().start(DPS::ExchangingTime);

    // Applying imported clauses involves propagations that may cause new importation.
    // These clauses should be separated from new importation.
    std::vector<DPS::Clause> importedUnitClauses = std::move(wrapper->getImportedUnitClauses());
    std::vector<DPS::Clause> importedClauses = std::move(wrapper->getImportedClauses());

    bool ret = true;
    // First, applies unit clauses
    for (DPS::Clause& clause : importedUnitClauses) {
        int res = kissat_add_imported_unit_clause(solver, clause[0]);
        if (res == 1) continue;     // already satisfied
        wrapper->incNumImportedClauses();
        if (res == -1) {
            ret = false;
            break;
        }
    }
    importedUnitClauses.clear();
    // Then, applies non-unit clauses
    for (DPS::Clause& clause : importedClauses) {
        int res = kissat_add_imported_clause(solver, clause.data(), clause.size());
        if (res == 1) continue;     // already satisfied
        wrapper->incNumImportedClauses();
        if (res == -1) {
            ret = false;
            break;
        }
    }
    importedClauses.clear();
    wrapper->getChronometer().stop(DPS::ExchangingTime);

    return ret;
}

}