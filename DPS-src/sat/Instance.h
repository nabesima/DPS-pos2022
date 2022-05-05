#ifndef _DPS_INSTANCE_H_
#define _DPS_INSTANCE_H_

#include <vector>
#include <string>

#include "../sat/Clause.h"

namespace DPS {

class Instance : private std::vector<Clause> {      // because STL classes have no virtual destructor
protected:
    uint64_t num_vars;
    uint64_t num_clauses;
    uint64_t tot_literals;

public:
    // default constructor
    Instance() : vector(), num_vars(0), num_clauses(0), tot_literals(0) {}
    Instance(const Instance&) = default;            // copy constructor
    Instance& operator=(const Instance&) = default; // copy assignment operator
    Instance(Instance&&) = default;                 // move constructor
    Instance& operator=(Instance&&) = default;      // move aissgnment operator

    static Instance loadFormula(const std::string& filename);

    uint64_t getNumVars()       const { return num_vars; }
    uint64_t getNumClauses()    const { return num_clauses; }
    uint64_t getTotalLiterals() const { return tot_literals; }

    // provide some vector interfaces
    using vector::size;
    using vector::operator[];
    using vector::begin;
    using vector::end;
};

}

#endif