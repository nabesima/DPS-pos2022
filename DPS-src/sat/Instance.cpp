#include <iostream>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cassert>
#include <zlib.h>

#include "Instance.h"

using std::vector;

namespace DPS {

Instance Instance::loadFormula(const std::string& filename) {
	gzFile in = gzopen(filename.c_str(), "rb");
    if (in == Z_NULL) 
		throw std::invalid_argument("gzopen error: " + filename);

	Instance instance;
    Clause clause;
	int max_var = 0;
	bool neg = false;
	int c;
    while ((c = gzgetc(in)) != EOF) {
 		// comment or problem definition line
 		if (c == 'c' || c == 'p') {
 			// skip this line
 			while(c != '\n') 
                c = gzgetc(in);
 			continue;
 		}
 		// whitespace
 		if (isspace(c)) continue;
 		// negative?
 		if (c == '-') {
     		neg = true;
 			continue;
 		}
		// number
		if (isdigit(c)) {
			int num = c - '0';
			c = gzgetc(in);
			while (isdigit(c)) {
				num = num * 10 + (c - '0');
				c = gzgetc(in);
			}
			if (neg) {
                num = -num;
				neg = false;
			}
			if (num != 0) {
				clause.push_back(num);
				max_var = std::max(max_var, std::abs(num));				
                continue;
			} 
            instance.push_back(clause);
			instance.num_clauses++;
			instance.tot_literals += clause.size();
            clause.clear();
		}
	}
	instance.num_vars = max_var;

 	gzclose(in);	
	return instance;
}

}

