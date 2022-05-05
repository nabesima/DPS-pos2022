#include "ClauseBuffer.h"

namespace DPS {

void ClauseBuffer::addClause(const Clause& c, uint32_t val) {
    // 'val' is a new key
    if (keys.find(val) == keys.end()) 
        keys.insert(val);
    db[val].push_back(c);
    num_clauses++;
    num_literals += c.size();
}

uint32_t ClauseBuffer::exportTo(PrdClauses& dest, uint32_t max_lits) {
    uint32_t exported_literals = 0;
    uint32_t exported_clauses = 0;
    bool full = false;
    auto it = keys.begin();
    while (it != keys.end()) {
        std::deque<Clause>& queue = db[*it];
        while (!queue.empty()) {
            Clause& c = queue.front();
            dest.addClause(c);
            exported_literals += c.size();
            exported_clauses++;
            num_literals -= c.size();
            num_clauses--;
            queue.pop_front();
            // If max_lits == 0, it means unlimit. 
            if (0 < max_lits && max_lits < exported_literals) { 
                full = true;
                break;
            }
        }
        if (full) break;
        // if queue is a empty, then the key is removed.
        if (queue.empty()) {
            db.erase(*it);
            keys.erase(it++);
        }
        else
            ++it;
    }

    return exported_clauses;
}

std::ostream & operator << (std::ostream& stream, const ClauseBuffer& buf) {
    auto i = buf.keys.begin();
    while (i != buf.keys.end()) {
        stream << "Key " << *i << std::endl;
        const std::deque<Clause>& queue = buf.db.at(*i);
        auto j = queue.begin();
        while (j != queue.end()) {
            stream << "  " << *j << std::endl;
            ++j;
        }
        ++i;
    }
    return stream;
}

}