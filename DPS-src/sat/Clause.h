#ifndef _DPS_CLAUSE_H_
#define _DPS_CLAUSE_H_

#include <iostream>
#include <vector>

namespace DPS {

typedef std::vector<int> Clause;     // A Clause in DPS is a vector of int

std::ostream & operator << (std::ostream& stream, const Clause& c);

}

#endif