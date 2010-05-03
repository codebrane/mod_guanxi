#include "pod.h"

Pod::Pod(string pod_session_id) : session_id(pod_session_id) {
}

Pod::~Pod() {
}

string Pod::get_session_id(void) {
	return session_id.c_str();
}

void Pod::set_saml_response(string saml_response) {
	this->saml_response.assign(saml_response);
}

string Pod::get_saml_response(void) {
	return saml_response.c_str();
}

void Pod::set_attributes(const char* json) {
}
