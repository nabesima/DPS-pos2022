#ifndef _DPS_THREAD_LOCAL_VARS_H_
#define _DPS_THREAD_LOCAL_VARS_H_

#ifdef __cplusplus
#include <cstdint>
namespace DPS {
    extern "C" {
#else
#define thread_local _Thread_local
#endif

        extern thread_local uint64_t num_mem_accesses;

#ifdef __cplusplus
    }
}
#endif

#endif
