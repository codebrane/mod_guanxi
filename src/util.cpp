#include <time.h>
#include <sstream>
#include <vector>

#include "util.h"
#include "pod.h"

string Util::generateUUID() {
	srand(time(NULL));
	stringstream out;
	out << rand();
	return out.str();
}

const char* Util::getServiceName(char* uri) {
	char buffer[50];
	char* token = strtok(uri, "/");
	while (token != NULL) {
		strcpy(buffer, token);
		token = strtok(NULL, "/");
	}
	char* service = new char[strlen(buffer)];
	strcpy(service, buffer);
	return (const char*)service;
}

// https://www.securecoding.cert.org/confluence/display/seccode/STR06-C.+Do+not+assume+that+strtok%28%29+leaves+the+parse+string+unchanged
const char* Util::getRequestParam(const char* paramName, const char* params) {
	if (params == NULL) {
		return NULL;
	}

	char buffer[strlen(params) + 2];
	strcpy(buffer, params);
	strcat(buffer, "&");

	vector<char*> namesAndValues;
	char* token = strtok(buffer, "&"); // token: name1=value1
	while (token != NULL) {
		namesAndValues.push_back(token);
		token = strtok(NULL, "&");
	}

	vector<char*>::const_iterator iterator;
	for (iterator = namesAndValues.begin(); iterator != namesAndValues.end(); iterator++) {
		token = strtok(((char*)(*iterator)), "="); // token: namex
		if (strcmp(token, paramName) == 0) {
			token = strtok(NULL, "="); // token: valuex
			char* paramValue = new char[strlen(token)];
			strcpy(paramValue, token);
			return (const char*)paramValue;
		}
	}

	return NULL;
}
