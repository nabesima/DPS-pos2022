#ifndef _DPS_MODEL_H_
#define _DPS_MODEL_H_

#include <iostream>
#include <vector>
#include <cstdint>

#include "Clause.h"
#include "Instance.h"

namespace DPS {

class Model : private std::vector<bool> {      // because STL classes have no virtual destructor
protected:
    uint64_t num_vars;
public:
    Model(uint64_t vars) : num_vars(vars) { resize(vars + 1); }  // ignore index 0
    Model(const Model&) = default;            // copy constructor
    Model& operator=(const Model&) = default; // copy assignment operator
    Model(Model&&) = default;                 // move constructor
    Model& operator=(Model&&) = default;      // move aissgnment operator

    bool value(const int lit) const;
    bool satisfies(const Instance& instance) const;
    bool satisfies(const Clause& clause) const;

    // provide some vector interfaces
    using vector::size;
    using vector::operator[];
    using vector::begin;
    using vector::end;
};

std::ostream & operator << (std::ostream& stream, const Model& m);

}

#endif