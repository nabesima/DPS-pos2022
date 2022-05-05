/******************************************************************************
 Copyright (c) 2020- Hidetomo NABESHIMA, Tsubasa FUKIAGE, Yuto OBITSU, 
                                                University of Yamanashi, Japan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <iostream>
#include <signal.h>

#include "parallel/DetParallelSolver.h"
#include "parallel/Version.h"
#include "utils/System.h"

using std::cout;
using std::endl;
using namespace DPS;

//=================================================================================================
// Main:

static DetParallelSolver* dps_solver = nullptr;
static double start_real_time = 0;

static void SIGINT_exit(int signum) {
    if (dps_solver) dps_solver->printResult();
    cout << "c" << endl;
    cout << "c ********** INTERRUPTED **********" << endl;
    cout << "c Signal " << signum << " (" << std::string(strsignal(signum)) << ") catched" << endl;
    cout << "c" << endl;
    if (dps_solver) _exit(dps_solver->getResult());
    _exit(0); 
}

int main(int argc, char** argv) {
    try {
        start_real_time = realTime();
        
        DetParallelSolver dps;
        dps_solver = &dps;
        dps.setOptions(argc, argv);    

        // Use signal handlers that forcibly quit until the solver will be able to respond to
        // interrupts:
        signal(SIGINT,  SIGINT_exit);
        signal(SIGXCPU, SIGINT_exit);

        if (dps.verbose()) {
            cout << "c DPS (Deterministic Parallel SAT Solver) version " << VERSION << endl;
            cout << "c " << COPYRIGHT << endl;
            cout << "c " << BUILD_INFO << endl;

            cout << "c Command: " << argv[0];
            for (int i=1; i < argc; i++)
                cout << ' ' << argv[i];
            cout << endl;
        }

        SATResult ret = dps.solve();
        exit(ret);

    } catch (std::exception& e) {
        if (dps_solver) dps_solver->printResult();
        cout << "c" << endl;
        cout << "c ********** EXCEPTION OCCURED **********" << endl;
        cout << "c Error: " << e.what() << endl;
        cout << "c" << endl;
        if (dps_solver) _exit(dps_solver->getResult()); 
        _exit(0); 
    }
}