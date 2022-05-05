#include <iostream>
#include <string>

// Include files should be specified relatively to avoid confusion with same named files.
#include "Options.h"
#include "Version.h"

namespace DPS {

using std::cout;
using std::endl;
using std::string;

cxxopts::Options Options::makeDefaultOptions() {
    cxxopts::Options options("DPS", "Deterministic Parallel SAT solver version " + VERSION);
    options
        .positional_help("<dimacs>")
        .show_positional_help();    
    
    options.add_options()
        ("h,help",        "print this list of all command line options")
        ("i,input",       "input dimacs file",                   cxxopts::value<std::string>())
        ("model",         "show model when SAT",                 cxxopts::value<bool>()->default_value("false"))
        ("verify",        "verify model",                        cxxopts::value<bool>()->default_value("false"))
        ("real-time-lim", "real time limit (0 for unlimited)",   cxxopts::value<double>()->default_value("0"), "N")
        ("mem-lim",       "memory limit [MB] (0 for unlimited)", cxxopts::value<double>()->default_value("0"), "N")
        ("banner",        "print solver information",            cxxopts::value<bool>()->default_value("false"))
        ("log-interval",  "interval of log output",              cxxopts::value<uint32_t>()->default_value("10"), "N")
        ("v,verbose",     "verbose level",                       cxxopts::value<uint32_t>()->default_value("1"), "N")
        ("q,quiet",       "quiet mode",                          cxxopts::value<bool>()->default_value("false"))
    ;

    options.add_options("Parallel solving")
        ("s,solver",       "base solver name (minisat/glucose/mcomsps/kissat)",  cxxopts::value<string>()->default_value("kissat"), "NAME")
        ("n,nthreads",     "number of threads (0 for automatic)",                cxxopts::value<uint32_t>()->default_value("0"),       "N")
        ("m,margin",       "margin for delayed clause exchange",                 cxxopts::value<uint32_t>()->default_value("20"),      "N")
        ("p,period",       "# of memory accesses for a period",                  cxxopts::value<uint64_t>()->default_value("1000000"), "N")
        ("adjust-threads", "adjust # of threads to avoid consuming all memory s.t. init memory size * N * # of threads <= mem-lim (0 as no-adjust)",
                                                                   cxxopts::value<uint32_t>()->default_value("3"), "N")
        ("fapp-clauses",   "imported clauses limit for forced application (0 for unlimited)", 
                                                                   cxxopts::value<uint32_t>()->default_value("10000"), "N")
        ("fapp-periods",   "minimum period-interval for forced application (0 for unlimited)", 
                                                                   cxxopts::value<uint32_t>()->default_value("50"), "N")
        ("adpt-prd",       "adaptive period-length strategy (specify # of conflicts / period, 0 means unuse)",
                                                                   cxxopts::value<uint32_t>()->default_value("0"), "N")
        ("adpt-prd-lb",    "lb of adaptive period-length (specify # of mem accs, 0 means unuse)",
                                                                   cxxopts::value<uint64_t>()->default_value("0"), "N")
        ("adpt-prd-ub",    "ub of adaptive period-length (specify # of mem accs, 0 means unuse)",
                                                                   cxxopts::value<uint64_t>()->default_value("0"), "N")
        ("adpt-prd-smth",  "smoothing factor of adaptive period-length strategy",
                                                                   cxxopts::value<double>()->default_value("0.1"), "N")
        ("exp-lbdq-lim",   "LBD upperbound to be exported (specify quantile of LBD distribution)",
                                                                   cxxopts::value<double>()->default_value("0.2"), "N")
        ("exp-lits-lim",   "threshold for number of exported ltierals (0 for unlimited)",
                                                                   cxxopts::value<uint32_t>()->default_value("150"), "N")
        ("exp-lits-margin", "allowable margin of exported ltierals",
                                                                   cxxopts::value<double>()->default_value("3"), "N")
    ;

    options.add_options("SAT solver - MiniSAT")
        ("ms-len",  "upper-bound of clause length for export (0 for unlimited)",            cxxopts::value<uint32_t>()->default_value("10"), "N")        
        ("ms-simp", "enable preprocesing of MiniSAT",                                       cxxopts::value<bool>()->default_value("false"))        
    ;

    options.add_options("SAT solver - Glucose")
        ("gl-lbd",  "upper-bound of clause LBD for export (0 for unlimited)",               cxxopts::value<uint32_t>()->default_value("7"), "N")        
        ("gl-len",  "upper-bound of clause length for export (0 for unlimited)",            cxxopts::value<uint32_t>()->default_value("24"), "N")        
        ("gl-simp", "enable preprocesing of Glucose",                                       cxxopts::value<bool>()->default_value("false"))        
    ;

    options.add_options("SAT solver - MapleCOMSPS")
        ("mc-lbd",  "initial upper-bound of clause LBD for export (0 for unlimited)",       cxxopts::value<uint32_t>()->default_value("3"), "N")        
        ("mc-len",  "upper-bound of clause length for export (0 for unlimited)",            cxxopts::value<uint32_t>()->default_value("0"), "N")        
        ("mc-simp", "enable preprocesing of MapleCOMSPS",                                   cxxopts::value<bool>()->default_value("false"))        
    ;

    options.add_options("SAT solver - Kissat")
        ("ks-lbd",      "initial upper-bound of clause LBD for export (0 for unlimited)",   cxxopts::value<uint32_t>()->default_value("3"), "N")        
        ("ks-stable",   "stable mode search (0: focus, 1: alt, 2: stable, 4: mix)",         cxxopts::value<uint32_t>()->default_value("1"), "N")        
        ("ks-elim",     "percentage of threads applying elimination",                       cxxopts::value<uint32_t>()->default_value("50"))
        ("ks-pakis",    "use PaKis search parameters for diversification",                  cxxopts::value<bool>()->default_value("false"))
    ;

    options.parse_positional({"input"});
    
    return options;
}

static const char* const default_argv[] = { "DPS" };
void Options::setOptions(int argc, const char* const argv[]) {
    auto options = makeDefaultOptions();
    cxxopts::ParseResult result;
    try {        
        if (argv != nullptr) 
            result = options.parse(argc, argv);
        else
            result = options.parse(1, default_argv);
    } catch (cxxopts::OptionException& e) {
        cout << "Error: " << e.what() << " (try '-h')" << endl;
        exit(0);
    }    
    if (result.count("help")) {
        cout << options.help() << endl;
        exit(0);
    }
    if (result.unmatched().size() > 0) {
        for (const auto& arg : result.unmatched())
            cout << "Error: unsupported argument '" << arg << "' (try '-h')" << endl;      
        exit(0);
    }
    if (result.count("banner")) {
        cout << "DPS (Deterministic Parallel SAT Solver) version " << VERSION << endl;
        cout << COPYRIGHT << endl;
        cout << BUILD_INFO << endl;
        exit(0);
    }
    
    // apply parsed options
    if (result.count("input")) setInputFile(result["input"].as<string>());
    setShowModel    (result["model"          ].as<bool>());
    setVerifyModel  (result["verify"         ].as<bool>());
    setRealTimeLim  (result["real-time-lim"  ].as<double>());
    setMemUseLim    (result["mem-lim"        ].as<double>());
    setLogInterval  (result["log-interval"   ].as<uint32_t>());
    setVerboseLv    (result["quiet"          ].as<bool>() ? 0 : result["verbose"].as<uint32_t>());
    setNumThreads   (result["nthreads"       ].as<uint32_t>());
    setMargin       (result["margin"         ].as<uint32_t>());
    setMemAccLim    (result["period"         ].as<uint64_t>());
    setBaseSolver   (result["solver"         ].as<string>());
    setAdjustThreads(result["adjust-threads" ].as<uint32_t>());
    setFAppClauses  (result["fapp-clauses"   ].as<uint32_t>());
    setFAppPeriods  (result["fapp-periods"   ].as<uint32_t>());
    setAdptPrd      (result["adpt-prd"       ].as<uint32_t>());
    setAdptPrdLB    (result["adpt-prd-lb"    ].as<uint64_t>());
    setAdptPrdUB    (result["adpt-prd-ub"    ].as<uint64_t>());
    setAdptPrdSmth  (result["adpt-prd-smth"  ].as<double>());
    setExpLBDQLim   (result["exp-lbdq-lim"   ].as<double>());
    setExpLitsLim   (result["exp-lits-lim"   ].as<uint32_t>());
    setExpLitsMargin(result["exp-lits-margin"].as<double>());
    setMSLenLim     (result["ms-len"         ].as<uint32_t>());
    setMSSimp       (result["ms-simp"        ].as<bool>());
    setGLLBDLim     (result["gl-lbd"         ].as<uint32_t>());
    setGLLenLim     (result["gl-len"         ].as<uint32_t>());
    setGLSimp       (result["gl-simp"        ].as<bool>());
    setMCLBDLim     (result["mc-lbd"         ].as<uint32_t>());
    setMCLenLim     (result["mc-len"         ].as<uint32_t>());
    setMCSimp       (result["mc-simp"        ].as<bool>());
    setKSLBDLim     (result["ks-lbd"         ].as<uint32_t>());
    setKSStable     (result["ks-stable"      ].as<uint32_t>());
    setKSElim       (result["ks-elim"        ].as<uint32_t>());
    setKSPaKis      (result["ks-pakis"       ].as<bool>());
}

void Options::printOptions() {
    cout << "c [Options]" << endl;
    cout << "c  base solver      = " << base_solver << endl;
    cout << "c  num threads      = " << num_threads << endl;
    cout << "c  margin           = " << margin << endl;
    cout << "c  period           = " << mem_acc_lim << endl;
    cout << "c  adjust threads   = " << adjust_threads << endl;
    cout << "c  fapp clauses     = " << fapp_clauses << endl;
    cout << "c  fapp periods     = " << fapp_periods << endl;
    cout << "c  adpt prd         = " << adpt_prd << endl;
    cout << "c  adpt prd lb      = " << adpt_prd_lb << endl;
    cout << "c  adpt prd ub      = " << adpt_prd_ub << endl;
    cout << "c  adpt prd smth    = " << adpt_prd_smth << endl;
    cout << "c  exp lbdq lim     = " << exp_lbdq_lim << endl;
    cout << "c  exp lits lim     = " << exp_lits_lim << endl;
    cout << "c  exp lits margin  = " << exp_lits_margin << endl;
    cout << "c  ms len           = " << ms_len_lim << endl;
    cout << "c  ms simp          = " << ms_simp << endl;
    cout << "c  gl lbd           = " << gl_lbd_lim << endl;
    cout << "c  gl len           = " << gl_len_lim << endl;
    cout << "c  gl simp          = " << gl_simp << endl;
    cout << "c  mc lbd           = " << mc_lbd_lim << endl;
    cout << "c  mc len           = " << mc_len_lim << endl;
    cout << "c  mc simp          = " << mc_simp << endl;
    cout << "c  ks lbd           = " << ks_lbd_lim << endl;
    cout << "c  ks stable        = " << ks_stable << endl;
    cout << "c  ks elim          = " << ks_elim << endl;
    cout << "c  ks pakis         = " << ks_pakis << endl;
    cout << "c  real time lim    = " << real_time_lim << endl;
    cout << "c  memory lim       = " << mem_use_lim << endl;
    cout << "c" << endl;
}

}