#include <iostream>
#include <atomic>
#include <stdexcept>

#include "SolverFactory.h"
#include "MiniSatWrapper.h"
// To avoid redefining the following constants
#undef l_True
#undef l_False
#undef l_Undef
#include "GlucoseWrapper.h"
// To avoid redefining the following constants
#undef l_True
#undef l_False
#undef l_Undef
#include "MapleCOMSPSWrapper.h"
// To avoid redefining the following constants
#undef l_True
#undef l_False
#undef l_Undef
#include "KissatWrapper.h"

using namespace DPS;

static std::atomic<int> currentIdSolver(0);

std::vector<AbstDetSeqSolver*> SolverFactory::createSATSolvers(
   int num_solvers, Sharer *sharer, Options& options) {
   std::vector<AbstDetSeqSolver*> solvers;
   for (int i=0; i < num_solvers; i++) 
      solvers.push_back(SolverFactory::createSATSolver(options.getBaseSolver(), sharer, options));   
   return solvers;
}

AbstDetSeqSolver* SolverFactory::createSATSolver(std::string name, Sharer *sharer, Options& options) {
   int id = currentIdSolver.fetch_add(1);
   AbstDetSeqSolver *solver = nullptr;
   if (name == "minisat") 
      solver = new MiniSatWrapper(id, sharer, options);
   else if (name == "glucose") 
      solver = new GlucoseWrapper(id, sharer, options);   
   else if (name == "mcomsps")
      solver = new MapleCOMSPSWrapper(id, sharer, options);
   else if (name == "kissat")
      solver = new KissatWrapper(id, sharer, options
            // dps->getMargin(), dps->getMemAccLim(), 
            // dps->getFAppClauses(), dps->getFAppPeriods(), dps->getExpLitsLim(), dps->getExpLitsMargin(),
            // dps->getKSLBDLim(), dps->getKSStable()
         );
   else 
      throw std::runtime_error("Error: unknown solver name'" + name + "'");

   if (!solver) throw std::runtime_error("could not allocate memory for Wrapper: " + name);

   return solver;
}
