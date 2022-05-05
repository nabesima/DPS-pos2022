#ifndef _DPS_CLAUSE_BUFFER_H_
#define _DPS_CLAUSE_BUFFER_H_

#include <iostream>
#include <unordered_map>
#include <deque>
#include <set>

#include "../sat/Clause.h"
#include "PrdClauses.h"

namespace DPS {

class ClauseBuffer {
private:
    std::unordered_map<uint32_t, std::deque<Clause>> db;
    std::set<uint32_t> keys;
    uint32_t num_clauses;
    uint64_t num_literals;

public:
    ClauseBuffer() : num_clauses(0), num_literals(0) {}
    void addClause(const Clause& c, uint32_t val);
    uint32_t exportTo(PrdClauses& dest, uint32_t max_lits);
    
    uint32_t getNumClauses() { return num_clauses; }
    uint64_t getNumLiterals() { return num_literals; }

    friend std::ostream & operator << (std::ostream& stream, const ClauseBuffer& buf);
};

}
#endif
