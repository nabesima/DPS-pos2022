#include <cassert>
#include "Model.h"

namespace DPS {

bool Model::satisfies(const Instance& instance) const {
    for (const Clause& clause : instance)
        if (!satisfies(clause))
            return false;
    return true;
}

bool Model::satisfies(const Clause& clause) const {
    for (const int lit : clause) 
        if (value(lit))
            return true;
    return false;
}

bool Model::value(const int lit) const {
    uint32_t var = std::abs(lit);    
    assert(0 < var && var < size());
    bool b = (*this)[var];
    return lit > 0 ? b : !b;
}

template <class T> 
static uint32_t numDigits(T number) {
    int digits = (number < 0) ? 1 : 0;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

std::ostream & operator << (std::ostream& stream, const Model& model) {
   uint32_t used_width = 0;
   for (size_t var=1; var < model.size(); var++) {
       int val = model[var] ? +var : -var;
        uint32_t digits = numDigits(val);
        if (used_width + 1 + digits >= 80) {
            stream << std::endl;
            used_width = 0;
        }
        if (used_width == 0) {
            stream << 'v';
            used_width++;
        }
        stream << ' ' << val;
        used_width += digits + 1;
    }
    return stream << " 0" << std::endl;
}

}