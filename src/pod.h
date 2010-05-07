#ifndef POD_H_
#define POD_H_

#include <string>

using namespace std;

/// Class that represents attributes and their assertion
/**
 * A Pod contains attributes for a user as well as the original
 * SAML assertion that they came from.
 *
 * \author alistair
 */
class Pod {
public:
	/// Default constructor
	Pod(string podSessionID);

	/// Default destructor
	virtual ~Pod();

	/// Stores the original SAML response from whence the attributes came
	/**
	 * This is the original SAML response that contains the attributes
	 * in their security context
	 * \param saml_response The SAML Response containing the attributes
	 */
	void setSAMLResponse(string samlResponse);

	/// Stores the attributes sent from the Guanxi SAML Engine
	/**
	 * This parses the incoming JSON from the Engine and stores the
	 * attributes in an easily accessible format
	 * \param json The JSON from the Guanxi SAML Engine containing the attributes
	 */
	void setAttributes(const char* json);

	/// Returns the session id this Pod is tied to
	/**
	 * Each Pod is tied to a particular session id. This gets it
	 * \return the session id of the Pod
	 */
	string getSessionID(void);

	/// Returns the original SAML Response containing the attributes
	/**
	 * This retrieves the original SAML Response containing the attributes
	 * in their security context
	 * \return the original SAML Response as string version of the XML
	 */
	string getSAMLResponse(void);

	/// Gets the original URI that was requested
	/**
	 * This returns the original URI that the Pod was created for
	 * e.g. /test/index.html
	 * \return the uri
	 */
	string getURI(void);

	/// Sets the URI for the Pod
	/**
	 * This is the original URI that was requested
	 * \param uri the original URI
	 */
	void setURI(string uri);

private:
	/// The session id of the Pod
	string sessionID;
	/// The SAML Response containing the attributes in their security context
	string samlResponse;
	/// The original uri for which the Pod was created
	string uri;
};

#endif /* POD_H_ */
