#include "pod.h"

Pod::Pod(string podSessionID) : sessionID(podSessionID) {
}

Pod::~Pod() {
}

string Pod::getSessionID(void) {
	return sessionID;
}

void Pod::setSAMLResponse(string samlResponse) {
	this->samlResponse.assign(samlResponse);
}

string Pod::getSAMLResponse(void) {
	return sessionID;
}

void Pod::setAttributes(const char* json) {
}

string Pod::getURI(void) {
	return uri;
}

void Pod::setURI(string podURI) {
	uri = podURI;
}

