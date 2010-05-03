#include "pod.h"

Pod::Pod(string podSessionID) : sessionID(podSessionID) {
}

Pod::~Pod() {
}

string Pod::getSessionID(void) {
	return sessionID.c_str();
}

void Pod::setSAMLResponse(string samlResponse) {
	this->samlResponse.assign(samlResponse);
}

string Pod::getSAMLResponse(void) {
	return sessionID.c_str();
}

void Pod::setAttributes(const char* json) {
}
