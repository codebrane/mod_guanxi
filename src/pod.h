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
	Pod(string pod_session_id);

	/// Default destructor
	virtual ~Pod();

	/// Stores the original SAML response from whence the attributes came
	/**
	 * This is the original SAML response that contains the attributes
	 * in their security context
	 * \param saml_response The SAML Response containing the attributes
	 */
	void set_saml_response(string saml_response);

	/// Stores the attributes sent from the Guanxi SAML Engine
	/**
	 * This parses the incoming JSON from the Engine and stores the
	 * attributes in an easily accessible format
	 * \param json The JSON from the Guanxi SAML Engine containing the attributes
	 */
	void set_attributes(const char* json);

	/// Returns the session id this Pod is tied to
	/**
	 * Each Pod is tied to a particular session id. This gets it
	 * \return the session id of the Pod
	 */
	string get_session_id(void);

	/// Returns the original SAML Response containing the attributes
	/**
	 * This retrieves the original SAML Response containing the attributes
	 * in their security context
	 * \return the original SAML Response as string version of the XML
	 */
	string get_saml_response(void);

private:
	/// The session id of the Pod
	string session_id;
	/// The SAML Response containing the attributes in their security context
	string saml_response;
};

#endif /* POD_H_ */
