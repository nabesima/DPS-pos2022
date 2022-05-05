#include "Clause.h"

namespace DPS {

std::ostream & operator << (std::ostream& stream, const Clause& c) {
    for (auto lit : c)
        stream << lit << ' ';
    return stream ;
}

}