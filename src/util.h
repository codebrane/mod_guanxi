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

	/// Retrieves the name of the service being accessed
	/**
	 * Retrieves the last part of the request uri
	 * \param uri the request uri e.g. /test/testservice
	 * \return the last part of the request uri e.g. testservice
	 */
	static const char* getServiceName(char* uri);

	/// Gets the value of a request parameter
	/**
	 * Retrieves the value of a parameter from the request
	 * \param paramName the name of the parameter who's value you want
	 * \param the query string from the request
	 * \return the value of the parameter or NULL if the param doesn't exist
	 */
	static const char* getRequestParam(const char* paramName, const char* params);
};

#endif /* UTIL_H_ */
