#include "podmanager.h"

PodManager::PodManager() {
}

PodManager::~PodManager() {
}

void PodManager::add_pod(string session_id) {
	Pod* pod = new Pod(session_id);
	pods.push_back(pod);
}

Pod* PodManager::get_pod(string session_id) {
	vector<Pod*>::const_iterator iterator;

	for (iterator = pods.begin(); iterator != pods.end(); iterator++) {
		if (((Pod*)(*iterator))->get_session_id() == session_id) {
			return ((Pod*)(*iterator));
		}
	}

	return NULL;
}
