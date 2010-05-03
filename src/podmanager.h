#ifndef PODMANAGER_H_
#define PODMANAGER_H_

#include <vector>
#include "pod.h"

using namespace std;

/// Class for managing Pods of attributes
/**
 * The PodManager is responsible for managing Pods of attributes and
 * assertions, which are tied to cookie values. Once a user has
 * authenticated at their IdP and we have all their attributes via
 * the AttributeConsumerService, there will be a Pod tied to that
 * cookie value.
 *
 * \author alistair
 */
class PodManager {
public:
	/// Default constructor
	PodManager();

	/// Default destructor
	virtual ~PodManager();

	/// Instructs the PodManager to add a new Pod tied to the specified session id
	/**
	 * \param sessionID the cookie value which should be unique among the Pods
	 */
	void addPod(string sessionID);

	/// Retrieves a Pod tied to the specified session id or NULL if it doesn't exist
	/**
	 * \param session_id the cookie value which should be unique among the Pods
	 * \return the Pod for the session id or NULL if one doesn't exist
	 */
	Pod* getPod(string sessionID);

private:
	/// Internal list of Pods
	vector<Pod*> pods;
};

#endif /* PODMANAGER_H_ */
