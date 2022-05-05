#include <iostream>
#include <pthread.h>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <vector>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

// Include files should be specified relatively to avoid confusion with same named files.
#include "DetParallelSolver.h"
#include "../solvers/SolverFactory.h"
#include "../sat/Instance.h"
#include "../utils/System.h"

using namespace DPS;
using std::cout;
using std::endl;

DetParallelSolver::DetParallelSolver() :
    sharer(NULL)
,   num_threads(0)    
,   start_real_time(0.0)
,   num_print_stats(0)
{
    pthread_mutex_init(&mfinished, NULL); //PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_init(&cfinished, NULL);
}

static void *localLaunch(void *arg) {
    AbstDetSeqSolver *s = (AbstDetSeqSolver *) arg;
    
    s->getSharer()->incNumLiveThreads();
    s->solve();
    s->getSharer()->decNumLiveThreads();

    pthread_exit(NULL);
}

DetParallelSolver::~DetParallelSolver() {
    for (auto solver : solvers)
        delete solver;
    if (sharer) 
        delete sharer;
}

SATResult DetParallelSolver::solve() {
    start_real_time = realTime();
    
    // load input formula
    string input_file = options.getInputFile();
    if (input_file.size() > 0) {
        double start = realTime();        

        input_formula = Instance::loadFormula(input_file);        
        used_mem_after_loading = usedMemory();

        if (options.verbose()) {
            printf("c Loading '%s' (%.2f s)\n", input_file.c_str(), realTime() - start);
            cout << "c" << endl;
            if (options.verbose() >= 2) {
                cout << "c [Input formula]" << endl;
                cout << "c  num variables    = " << input_formula.getNumVars() << endl;
                cout << "c  num clauses      = " << input_formula.getNumClauses() << endl;
                cout << "c  total literlas   = " << input_formula.getTotalLiterals() << endl;
                cout << "c  curr used memory = " << used_mem_after_loading << " MB" << endl;
                cout << "c" << endl;
            }
        }
    }
    
    // generate solver objects
    if (solvers.size() == 0) 
        generateAllSolvers();
    assert(solvers.size() > 0);

    if (options.verbose() >= 2) 
        options.printOptions();    

    if (input_file.size() > 0) {
        // set input formula to each solver    
        for (auto solver : solvers)
            solver->setInputFormula(&input_formula);    // TODO: want to remove input_formula after all solver start to solve.
    }

    // Initialize and set thread detached attribute 
    pthread_attr_t thAttr;
    pthread_attr_init(&thAttr);
    pthread_attr_setdetachstate(&thAttr, PTHREAD_CREATE_JOINABLE);

    for(uint32_t i=0; i < num_threads; i++) {
        pthread_t *pt = (pthread_t *) malloc(sizeof(pthread_t));
        threads.push_back(pt);
        pthread_create(threads[i], &thAttr , &localLaunch, solvers[i]);
    }

    if (options.verbose())
        cout << "c Launched " << num_threads << " solvers" << endl;

    while (true) {
        if (sharer->getNumLiveSolvers() == 0) 
            break;

        struct timespec to;
        timespec_get(&to, TIME_UTC);
        to.tv_sec = time(nullptr) + options.getLogInterval();

        if (pthread_cond_timedwait(&cfinished, &mfinished, &to) != ETIMEDOUT)
            break;
        
        if (verbose())
            printStats();   
    }

    for (uint32_t i=0; i < num_threads; i++) { // Wait for all threads to finish
        pthread_join(*threads[i], NULL);
        free(threads[i]);       
    }

    printResult();

    return getResult();
}

void DetParallelSolver::generateAllSolvers() {   
    // use # of (logical) CPUs if unspecified
    num_threads = options.getNumThreads();
    if (options.getNumThreads() == 0)
        num_threads = std::thread::hardware_concurrency();

    // adjust # of threads to avoid consuming all memory
    uint32_t adjust_threads = options.getAdjustThreads();
    uint32_t mem_use_lim    = options.getMemUseLim();
    if (adjust_threads > 0 && mem_use_lim > 0) {
        if (used_mem_after_loading >= 250) {    // adjust if input instance is somewhat large (heuristics!)
            // 'used_mem_after_loading * adjust_threads' means the estimated memory size for solving (heuristics!)
            uint32_t n = mem_use_lim / (used_mem_after_loading * adjust_threads);
            if (n == 0) n = 1;
            if (num_threads < n) n = num_threads;
            if (n != num_threads) {
                if (options.verbose()) {
                    cout << "c NOTE: # of threads is adjusted to " << n << " to avoid consuming all memory" << endl;
                    cout << "c" << endl;
                }
                num_threads = n;
            }
        }
    }

    // generates learnt clause exchanger
    sharer = new Sharer(num_threads, options.getMargin(), options.getMemAccLim());
    if (!sharer) throw std::runtime_error("could not allocate memory for Sharer");
    
    // generates sub-solvers
    solvers = SolverFactory::createSATSolvers(num_threads, sharer, options);

    // set parameters to each solver
    for (auto solver : solvers) {
        solver->pmfinished = &mfinished;
        solver->pcfinished = &cfinished;
    }
}

// void DetParallelSolver::loadFormula(const string& filename) {
//     for (auto solver : solvers)
//         solver->loadFormula(filename.c_str());   // if there are many threads, it's slow!
// }

void DetParallelSolver::printStats() {
    uint32_t verb = options.verbose();
    if (!verb) return;
    
    if (num_print_stats % 30 == 0) {
        printf("c\n");
        printf("c                                         "); if (verb >= 2) for(size_t i=0; i < solvers.size(); i++) printf("                 thread %2ld                  ", i); printf("\n"); 
        printf("c time           confs/s        wait      "); if (verb >= 2) for(size_t i=0; i < solvers.size(); i++) printf("   periods        redudants        exported "); printf("\n"); 
        printf("c      period-len      props/s        MB  "); if (verb >= 2) for(size_t i=0; i < solvers.size(); i++) printf("         conflicts         imported         "); printf("\n"); 
        printf("c\n");
    }
    num_print_stats++;

    uint64_t total_confs = 0;
    uint64_t total_props = 0;
    for(size_t i=0; i < solvers.size(); i++) {
        total_confs += solvers[i]->getNumConflicts();
        total_props += solvers[i]->getNumPropagations();
    }
    double real_time = realTime() - start_real_time;
    double cpu_time = cpuTime();
    uint64_t mem_acc_lim = solvers.size() > 0 ? solvers[0]->getMemAccLim() : 0;
    double confs_per_sec = total_confs / real_time;
    double props_per_sec = total_props / real_time;
    double wait   = (real_time - cpu_time / solvers.size()) / real_time * 100.0;
    double memory = usedMemory();

    printf("c %4d %9" PRIu64 " %5d %8d %2d%% %5d", 
        (int)real_time,
        mem_acc_lim,
        (int)confs_per_sec,
        (int)props_per_sec,
        (int)wait,
        (int)memory);
    if (verb >= 2) {
        for(size_t i=0; i < solvers.size(); i++) {
            AbstDetSeqSolver *s = solvers[i];
            printf(" | %c %5" PRIu64 " %8" PRIu64 " %8" PRIu64 " %8" PRIu64 " %6" PRIu64,
                s->getSolverState(),                
                s->getCurrPeriod(),
                s->getNumConflicts(),
                s->getNumRedundantClauses(),
                s->getNumImportedClauses(),
                s->getNumExportedClauses()
            );
        }
    }
    printf("\n");
    fflush(stdout);
}

void DetParallelSolver::printResult() {

    if (options.verbose() >= 2) {
        printf("c\n");
        printf("c [Basic stats]\n");
        printf("c Threads : %d\n", num_threads);        
        printf("c Margin : %" PRIu32 "\n", options.getMargin());
        printf("c Variables : %" PRIu64 "\n", input_formula.getNumVars());
        printf("c Clauses : %" PRIu64 "\n", input_formula.getNumClauses());
        printf("c TotalLiterals : %" PRIu64 "\n", input_formula.getTotalLiterals());
        if (sharer && sharer->getResult() != UNKNOWN)
            printf("c Winner : %d\n", sharer->getWinner());

        uint64_t total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumConflicts();
            printf("c Conflicts_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Conflicts_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumDecisions();
            printf("c Decisions_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Decisions_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumPropagations();
            printf("c Propagations_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Propagations_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumRestarts();
            printf("c Restarts_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Restarts_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getCurrPeriod();
            printf("c Periods_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Periods_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getMemAccLim();
            printf("c PeriodLength_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c PeriodLength_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumExportedClauses();
            printf("c Exported_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Exported_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumImportedClauses();
            printf("c Imported_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c Imported_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint64_t c = solvers[i]->getNumForcedApplications();
            printf("c ForcedApplication_%zu : %" PRIu64 "\n", i, c);
            total += c;
        }
        printf("c ForcedApplication_total : %" PRIu64 "\n", total);

        total = 0;
        for (size_t i=0; i < solvers.size(); i++) {
            uint32_t c = solvers[i]->getExpLBDthreshold();
            printf("c ExpLBDThreshold_%zu : %d\n", i, c);
            total += c;
        }
        printf("c ExpLBDThreshold_total : %" PRIu64 "\n", total);

        for (int q = 5; q <= 30; q += 5) {
            double sum = 0;
            for (size_t i=0; i < solvers.size(); i++) {
                double c = solvers[i]->getLBDQuantile((double)q / 100.0);
                printf("c LBD%02dQuantile_%zu : %.1f\n", q, i, c);
                sum += c;
            }
            printf("c LBD%02dQuantile_total : %.1f\n", q, sum);
        }

        printf("c\n");
        printf("c [Time stats for parallel proccessing]\n");

        double total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = solvers[i]->parchrono.getTime(RunningTime);
            printf("c RunningTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c RunningTime_total : %f\n", total_time);

        total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = solvers[i]->parchrono.getTime(WaitingTime);
            printf("c WaitingTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c WaitingTime_total : %f\n", total_time);

        total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = solvers[i]->parchrono.getTime(ExchangingTime);
            printf("c ExchangingTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c ExchangingTime_total : %f\n", total_time);

        total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = solvers[i]->parchrono.getTime(PeriodUpdateTime);
            printf("c PeriodProcTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c PeriodProcTime_total : %f\n", total_time);

        vector<double> sum_non_waiting_time(solvers.size());
        vector<double> sum_solve_time(solvers.size());
        for (size_t i=0; i < solvers.size(); i++) {
            sum_non_waiting_time[i] =
                    solvers[i]->parchrono.getTime(RunningTime)
                    + solvers[i]->parchrono.getTime(ExchangingTime)
                    + solvers[i]->parchrono.getTime(PeriodUpdateTime);
            sum_solve_time[i] =
                    sum_non_waiting_time[i]
                    + solvers[i]->parchrono.getTime(WaitingTime);
        }

        total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = sum_non_waiting_time[i];
            printf("c NonWaitingTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c NonWaitingTime_total : %f\n", total_time);

        total_time = 0.0;
        for (size_t i=0; i < solvers.size(); i++) {
            double t = sum_solve_time[i];
            printf("c SolvingTime_%zu : %f\n", i, t);
            total_time += t;
        }
        printf("c SolvingTime_total : %f\n", total_time);
    }

    if (options.verbose()) {
        printf("c\n");
        printf("c [System stats]\n");
        printf("c RealTime : %f\n", realTime() - start_real_time);
        printf("c CPUTime : %f\n", cpuTime());
        printf("c InitialMemory : %f\n", used_mem_after_loading);
        printf("c UsedMemory : %f\n", usedMemory());
        printf("c PeakMemory : %f\n", peakMemory());    
        printf("c\n");
    }

    switch (getResult()) {
        case SAT:     cout << "s SATISFIABLE"   << endl; break;
        case UNSAT:   cout << "s UNSATISFIABLE" << endl; break;
        case UNKNOWN: cout << "s UNKNOWN"       << endl; break;
    }

    bool show_model   = options.getShowModel();
    bool verify_model = options.getVerifyModel();
    if (getResult() == SAT && (show_model || verify_model)) {
        Model model = solvers[sharer->getWinner()]->getModel();
        if (show_model) 
            cout << model;
        if (verify_model) {
            if (model.satisfies(input_formula)) 
                cout << "c Verified: found model satisfies the input formula" << endl;
            else {
                cout << "c Error: found model does not satisfy the formula!" << endl;            
                exit(-1);
            }
        }
    }
    
    fflush(stdout);
}
