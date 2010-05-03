#include <time.h>
#include <sstream>

#include "util.h"

string Util::generate_uuid() {
	srand(time(NULL));
	stringstream out;
	out << rand();
	return out.str();
}
