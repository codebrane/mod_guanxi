#include "podmanager.h"

PodManager::PodManager() {
}

PodManager::~PodManager() {
}

void PodManager::addPod(string sessionID) {
	Pod* pod = new Pod(sessionID);
	pods.push_back(pod);
}

Pod* PodManager::getPod(string sessionID) {
	vector<Pod*>::const_iterator iterator;

	for (iterator = pods.begin(); iterator != pods.end(); iterator++) {
		if (((Pod*)(*iterator))->getSessionID() == sessionID) {
			return ((Pod*)(*iterator));
		}
	}

	return NULL;
}
