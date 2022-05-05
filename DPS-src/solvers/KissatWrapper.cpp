// Include files should be specified relatively to avoid confusion with same named files.
#include "KissatWrapper.h"

using namespace DPS;

struct PaKisParam {
    uint32_t tier1;
    uint32_t chrono;
    uint32_t stable;
    uint32_t walkinitially;
    uint32_t target;
    uint32_t phase;
};

static PaKisParam pakisParams[] = {
    { 2, 1, 1, 0, 1, 1 }, 
    { 2, 1, 1, 0, 2, 1 },
    { 2, 1, 0, 0, 1, 1 },
    { 2, 0, 1, 0, 1, 1 },
    { 2, 0, 1, 0, 1, 0 },
    { 2, 1, 1, 0, 1, 0 },
    { 2, 0, 2, 0, 1, 1 },
    { 2, 1, 1, 0, 0, 1 },
    { 2, 0, 1, 0, 0, 0 },
    { 2, 1, 1, 0, 0, 0 },
    { 2, 1, 1, 1, 1, 1 },
    { 2, 0, 1, 0, 2, 1 },
    { 2, 0, 1, 0, 2, 0 },
    { 3, 0, 1, 0, 2, 0 },
    { 3, 0, 1, 0, 2, 1 },
    { 2, 1, 1, 0, 2, 0 },
    { 2, 0, 2, 0, 2, 1 },
    { 2, 1, 1, 0, 0, 1 },
    { 2, 0, 1, 0, 0, 0 },
    { 2, 1, 1, 0, 0, 0 },
    { 3, 1, 1, 0, 0, 1 },
    { 3, 1, 1, 0, 2, 1 },
    { 2, 1, 1, 1, 2, 1 },
    { 2, 0, 0, 0, 1, 1 },
};

static int gcd(int a, int b) {
    if (a % b == 0)
        return b;
    else
        return gcd(b, a % b);
}

KissatWrapper::KissatWrapper(int id, Sharer *sharer, Options& options) : 
    AbstDetSeqSolver(id, sharer, options)
,   exp_clause_lbd_lim(options.getKSLBDLim())    
{
    solver = kissat_init();
    if (!solver) throw std::runtime_error("could not allocate memory for KissatWrapper::kissat");
    kissat_set_wrapper(solver, this, id);
    kissat_set_option(solver, "quiet", 1);

    // diversification strategy
    if (id) {
        kissat_set_option(solver, "seed", id);
        kissat_rand_pick_until_1st_conf(solver, true);
    }
    // stable
    if (options.getKSStable() < 4)  // 4 means mix mode
        kissat_set_option(solver, "stable", options.getKSStable());
    else
        kissat_set_option(solver, "stable", (id + 1) % 3);  // mix mode
    // eliminate
    if (options.getKSElim() != 100) {
        int num = options.getKSElim();
        int dem = 100;
        int div = gcd(num, dem);
        num = num / div;
        dem = dem / div;
        int elim = id % dem < num ? 1 : 0;
        kissat_set_option(solver, "eliminate", elim);
    }
    // pakis
    if (options.getKSPaKis()) {
        int i = id % (sizeof(pakisParams) / sizeof(PaKisParam));
        kissat_set_option(solver, "tier1",         pakisParams[i].tier1);
        kissat_set_option(solver, "chrono",        pakisParams[i].chrono);
        kissat_set_option(solver, "stable",        pakisParams[i].stable);
        kissat_set_option(solver, "walkinitially", pakisParams[i].walkinitially);
        kissat_set_option(solver, "target",        pakisParams[i].target);
        kissat_set_option(solver, "phase",         pakisParams[i].phase);
    }
}

KissatWrapper::~KissatWrapper() {
    kissat_release(solver);
}

bool KissatWrapper::shouldBeExported(uint32_t lbd) {
    lbd_dist.add(lbd);
    uint32_t threshold = getExpLBDthreshold();    
    return 0 < threshold && lbd <= threshold;
}

void KissatWrapper::uncheckedExportClause(const int *clause, uint32_t len, uint32_t lbd) {
    parchrono.start(ExchangingTime);
    exp_tmp.clear();
    const int *end = clause + len;
    for (const int *p = clause; p != end; p++)
        exp_tmp.push_back(*p);
    // export clause
    exp_clauses_buf.addClause(exp_tmp, exp_tmp.size());
    parchrono.stop(ExchangingTime);
}

SATResult KissatWrapper::solve() {
    if (input_formula != nullptr) {
        loadFormula(*input_formula);
        input_formula = nullptr;
    }

    parchrono.start(RunningTime);
    int res = kissat_solve(solver);
    parchrono.stop(RunningTime);
    SATResult result = UNKNOWN;
    if (res == 10) 
        result = SAT;
    else if (res == 20)
        result = UNSAT;
    sharer->IFinished(result, periods, thn);
    completeCurrPeriod();
    pthread_cond_signal(pcfinished);
    return result;
}

bool KissatWrapper::loadFormula(const Instance& clauses) {
    num_vars = clauses.getNumVars();
    kissat_reserve (solver, num_vars);
    for (auto clause : clauses) {
        for (auto n : clause) 
            kissat_add(solver, n);
        kissat_add(solver, 0);            
    }
    return true;
}

uint64_t KissatWrapper::getNumConflicts()        { return kissat_get_num_conflicts(solver); } 
uint64_t KissatWrapper::getNumDecisions()        { return kissat_get_num_decisions(solver); }
uint64_t KissatWrapper::getNumPropagations()     { return kissat_get_num_propagations(solver); }
uint64_t KissatWrapper::getNumRestarts()         { return kissat_get_num_restarts(solver); }
uint64_t KissatWrapper::getNumRedundantClauses() { return kissat_get_num_redudants(solver); }
char     KissatWrapper::getSolverState()         { return kissat_get_solver_state(solver); }

Model KissatWrapper::getModel() {
    Model model(num_vars);
    for (int var=1; var <= (int)num_vars; var++) {        
      int lit = kissat_value(solver, var);
      if (lit)
        model[var] = lit > 0;
   }
   return model;
}

// copied from kissat/src/handle.c
extern "C" {
    #include <assert.h>
    #include <signal.h>
    #include <stdbool.h>
    #include <stdio.h>

    static void (*handler) (int);
    static volatile int caught_signal;
    static volatile bool handler_set;

    #define SIGNALS \
    SIGNAL(SIGABRT) \
    SIGNAL(SIGBUS) \
    SIGNAL(SIGINT) \
    SIGNAL(SIGSEGV) \
    SIGNAL(SIGTERM)

    // *INDENT-OFF*

    #define SIGNAL(SIG) \
    static void (*SIG ## _handler)(int);
    SIGNALS
    #undef SIGNAL

    const char *
    kissat_signal_name (int sig)
    {
    #define SIGNAL(SIG) \
    if (sig == SIG) return #SIG;
    SIGNALS
    #undef SIGNAL
    if (sig == SIGALRM)
        return "SIGALRM";
    return "SIGUNKNOWN";
    }

    void
    kissat_reset_signal_handler (void)
    {
    if (!handler_set)
        return;
    #define SIGNAL(SIG) \
    signal (SIG, SIG ## _handler);
    SIGNALS
    #undef SIGNAL
    handler_set = false;
    handler = 0;
    }

    // *INDENT-ON*

    static void
    catch_signal (int sig)
    {
    if (caught_signal)
        return;
    caught_signal = sig;
    assert (handler_set);
    assert (handler);
    handler (sig);
    kissat_reset_signal_handler ();
    raise (sig);
    }

    void
    kissat_init_signal_handler (void (*h) (int sig))
    {
    assert (!handler);
    handler = h;
    handler_set = true;
    #define SIGNAL(SIG) \
    SIG ##_handler = signal (SIG, catch_signal);
    SIGNALS
    #undef SIGNAL
    }

    static volatile bool caught_alarm;
    static volatile bool alarm_handler_set;
    static void (*volatile SIGALRM_handler) (int);
    static void (*volatile handle_alarm) ();

    static void
    catch_alarm (int sig)
    {
    assert (sig == SIGALRM);
    if (caught_alarm)
        return;
    caught_alarm = true;
    static void (*volatile handler) ();
    handler = handle_alarm;
    if (!alarm_handler_set)
        raise (sig);
    assert (handler);
    handler ();
    }

    void
    kissat_init_alarm (void (*handler) (void))
    {
    assert (handler);
    assert (!caught_alarm);
    handle_alarm = handler;
    alarm_handler_set = true;
    assert (!SIGALRM_handler);
    SIGALRM_handler = signal (SIGALRM, catch_alarm);
    }

    void
    kissat_reset_alarm (void)
    {
    assert (alarm_handler_set);
    assert (handle_alarm);
    alarm_handler_set = false;
    handle_alarm = 0;
    (void) signal (SIGALRM, SIGALRM_handler);
    }
}