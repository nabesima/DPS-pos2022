#ifndef _DPS_OPTIONS_H_
#define _DPS_OPTIONS_H_

#include <string>

// Include files should be specified relatively to avoid confusion with same named files.
#include "../utils/cxxopts.hpp"

namespace DPS {

using std::string;    

class Options {
private:
    // basic options
    string      input_file;
    bool        show_model;
    bool        verify_model;
    double      real_time_lim;
    double      mem_use_lim;
    uint32_t    log_interval;
    uint32_t    verbose_lv;
    
    // parallel solving options
    string      base_solver;
    uint32_t    num_threads;
    uint32_t    margin;
    uint64_t    mem_acc_lim;
    bool        non_det;
    uint32_t    adjust_threads;
    uint32_t    fapp_clauses;
    uint32_t    fapp_periods;
    uint32_t    adpt_prd;
    uint64_t    adpt_prd_lb;
    uint64_t    adpt_prd_ub;
    double      adpt_prd_smth;
    double      exp_lbdq_lim;
    uint32_t    exp_lits_lim;
    double      exp_lits_margin;

    // MiniSAT options
    uint32_t    ms_len_lim;
    bool        ms_simp;

    // Glucose options
    uint32_t    gl_lbd_lim;
    uint32_t    gl_len_lim;
    bool        gl_simp;

    // MapleCOMSPS options
    uint32_t    mc_lbd_lim;
    uint32_t    mc_len_lim;
    bool        mc_simp;

    // Kissat options
    uint32_t    ks_lbd_lim;
    uint32_t    ks_stable;
    uint32_t    ks_elim;
    bool        ks_pakis;

    static cxxopts::Options makeDefaultOptions();

public:
    Options() { setOptions(); }

    void setOptions(int argc = 0, const char* const argv[] = nullptr);
    void printOptions();

    // basic options
    void          setInputFile(string s)            { input_file = s; }
    const string& getInputFile()              const { return input_file; }
    void          setShowModel(bool b)              { show_model = b; }
    bool          getShowModel()              const { return show_model; }
    void          setVerifyModel(bool b)            { verify_model = b; }
    bool          getVerifyModel()            const { return verify_model; }
    void          setRealTimeLim(double t)          { real_time_lim = t; }  
    double        getRealTimeLim()            const { return real_time_lim; }
    void          setMemUseLim(double t)            { mem_use_lim = t; }  
    double        getMemUseLim()              const { return mem_use_lim; }
    void          setLogInterval(uint32_t n)        { log_interval = n; }  
    uint32_t      getLogInterval()            const { return log_interval; }
    void          setVerboseLv(uint32_t n)          { verbose_lv = n; }
    uint32_t      getVerboseLv()              const { return verbose_lv; }
    uint32_t      verbose()                   const { return verbose_lv; }
    void          quiet()                           { verbose_lv = 0; }

    // parallel solving options
    void          setBaseSolver(string n)           { base_solver = n; }
    string        getBaseSolver()                   { return base_solver; }
    void          setNumThreads(uint32_t n)         { num_threads = n; }               
    uint32_t      getNumThreads()             const { return num_threads; }
    void          setMargin(uint32_t n)             { margin = n; }
    uint32_t      getMargin()                 const { return margin; }
    void          setMemAccLim(uint64_t n)          { mem_acc_lim = n; }
    uint64_t      getMemAccLim()              const { return mem_acc_lim; }
    void          setNonDetMode(bool b)             { non_det = b; }
    uint64_t      getNonDetMode()             const { return non_det; }
    void          setAdjustThreads(uint32_t n)      { adjust_threads = n; }
    uint32_t      getAdjustThreads()          const { return adjust_threads; }
    void          setFAppClauses(uint32_t n)        { fapp_clauses = n; }
    uint32_t      getFAppClauses()            const { return fapp_clauses; }
    void          setFAppPeriods(uint32_t n)        { fapp_periods = n; }
    uint32_t      getFAppPeriods()            const { return fapp_periods; }
    void          setAdptPrd(uint32_t n)            { adpt_prd = n; }
    uint32_t      getAdptPrd()                const { return adpt_prd; }
    void          setAdptPrdLB(uint64_t n)          { adpt_prd_lb = n; }
    uint64_t      getAdptPrdLB()              const { return adpt_prd_lb; }
    void          setAdptPrdUB(uint64_t n)          { adpt_prd_ub = n; }
    uint64_t      getAdptPrdUB()              const { return adpt_prd_ub; }
    void          setAdptPrdSmth(double d)          { adpt_prd_smth = d; }
    double        getAdptPrdSmth()            const { return adpt_prd_smth; }
    void          setExpLBDQLim(double d)           { exp_lbdq_lim = d; }
    double        getExpLBDQLim()             const { return exp_lbdq_lim; }
    void          setExpLitsLim(uint32_t n)         { exp_lits_lim = n; }
    uint32_t      getExpLitsLim()             const { return exp_lits_lim; }
    void          setExpLitsMargin(double d)        { exp_lits_margin = d; }
    double        getExpLitsMargin()          const { return exp_lits_margin; }

    // MiniSAT options
    void          setMSLenLim(uint32_t n)          { ms_len_lim = n; }
    uint32_t      getMSLenLim()              const { return ms_len_lim; }
    void          setMSSimp(bool b)                { ms_simp = b; }
    bool          getMSSimp()                const { return ms_simp; }

    // Glucose options
    void          setGLLBDLim(uint32_t n)          { gl_lbd_lim = n; }
    uint32_t      getGLLBDLim()              const { return gl_lbd_lim; }
    void          setGLLenLim(uint32_t n)          { gl_len_lim = n; }
    uint32_t      getGLLenLim()              const { return gl_len_lim; }
    void          setGLSimp(bool b)                { gl_simp = b; }
    bool          getGLSimp()                const { return gl_simp; }

    // MapleCOMSPS options
    void          setMCLBDLim(uint32_t n)          { mc_lbd_lim = n; }
    uint32_t      getMCLBDLim()              const { return mc_lbd_lim; }
    void          setMCLenLim(uint32_t n)          { mc_len_lim = n; }
    uint32_t      getMCLenLim()              const { return mc_len_lim; }
    void          setMCSimp(bool b)                { mc_simp = b; }
    bool          getMCSimp()                const { return mc_simp; }

    // Kissat options
    void          setKSLBDLim(uint32_t n)          { ks_lbd_lim = n; }
    uint32_t      getKSLBDLim()              const { return ks_lbd_lim; }
    void          setKSStable(uint32_t n)          { ks_stable = n; }
    uint32_t      getKSStable()              const { return ks_stable; }
    void          setKSElim(uint32_t n  )          { ks_elim = n; }
    uint32_t      getKSElim()                const { return ks_elim; }
    void          setKSPaKis(bool b)               { ks_pakis = b; }
    bool          getKSPaKis()               const { return ks_pakis; }
};

}

#endif