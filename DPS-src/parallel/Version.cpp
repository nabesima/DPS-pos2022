#include "Version.h"

namespace DPS {

const std::string COPYRIGHT = "Copyright (c) 2020-2022 H. Nabeshima, T. Fukiage, Y. Obitsu, X. Lu, K. Inoue";

#define STRINGIFY(x) #x 
#define VER_STRING(major, minor, patch) STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch)

const std::string VERSION = VER_STRING(DPS_MAJOR, DPS_MINER, DPS_PATCH);

const std::string CXX_COMPILER =
#ifdef __clang__
   "clang++"
#else
   "g++"
#endif
;

const std::string CXX_COMPILER_VER =
#ifdef __clang__
    VER_STRING(__clang_major__, __clang_minor__, __clang_patchlevel__)
#else
    VER_STRING(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#endif
;

const std::string BUILD_TYPE =
#ifdef DPS_BUILD_TYPE
    DPS_BUILD_TYPE
#else
    ""
#endif
;

const std::string BUILD_DATE = __DATE__ " " __TIME__;

const std::string BUILD_INFO = BUILD_TYPE + " build with " + CXX_COMPILER + " " + CXX_COMPILER_VER + " on " + BUILD_DATE;
}