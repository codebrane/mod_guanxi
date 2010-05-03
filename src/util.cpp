#include <time.h>
#include <sstream>

#include "util.h"
#include "pod.h"

string Util::generateUUID() {
	srand(time(NULL));
	stringstream out;
	out << rand();
	return out.str();
}

const char* Util::getCookieValue(request_rec* request, const char* cookie_name) {
	 return apr_table_get(request->headers_in, cookie_name);
}
