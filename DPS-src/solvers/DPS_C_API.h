#ifndef _DPS_C_INTERFACE_
#define _DPS_C_INTERFACE_

#ifdef __cplusplus
namespace DPS {
    extern "C" {
#endif

int DPS_checkPeriod(void *wrapper, const char *msg);
int DPS_shouldBeTerminated(void *wrapper);
int DPS_shouldApplyImportedClauses(void *wrapper);

bool DPS_kissat_shouldBeExported(void *wrapper, unsigned int lbd);
void DPS_kissat_uncheckedExportClause(void *wrapper, const int* clause, unsigned int size, unsigned int lbd);
int DPS_kissat_applyImportedClauses(void *wrapper);

#ifdef __cplusplus
    }
}
#endif

#endif