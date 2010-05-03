#ifndef UTIL_H_
#define UTIL_H_

#include <string>

#include "httpd.h"

using namespace std;

/// Utility class
/**
 * This class holds all the handy functions one could need in dealing
 * with SAML and other bits 'n bobs
 *
 * \author alistair
 */
class Util {
public:
	/// Generates a unique id
	/**
	 * This can be used to generate session ids for Pods
	 * \return a uuid
	 */
	static string generateUUID(void);

	static const char* getCookieValue(request_rec* request, const char* cookie_name);
};

#endif /* UTIL_H_ */
