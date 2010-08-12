/// mod_guanxi The Guanxi Guard Apache Module
/**
 * Guanxi Guard Apache Module
 *
 * \author alistair
 */
#include "mod_auth.h"
#include "ap_provider.h"
#include "http_request.h"
#include "http_core.h"

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"

#include "apr_strings.h"

#include "util.h"
#include "podmanager.h"
#include "attributeconsumerservice.h"

/// Large enough buffer to hold either the cookie name or its value when looking for the cookie
#define COOKIE_NAME_OR_VALUE_BUFFER_SIZE 100
/// Return this to the Engine's GPS service to verify the session
#define ENGINE_SESSION_VERIFIED "verified"
/// Return this to the Engine's GPS service to not verify the session
#define ENGINE_SESSION_NOTVERIFIED "notverified"

extern "C" module AP_MODULE_DECLARE_DATA mod_guanxi_module;

struct mod_guanxi_config {
	char* entityID;
	char* cookiePrefix;
	char* verifierURL;
	char* attributeConsumerURL;
	char* podderURL;
	char* engineGPSURL;
	PodManager* podManager;
	AttributeConsumerService* attributeConsumerService;
};

// Cookie GUANXI_GUARD_SERVICE_PROVIDER_mod-guanxi-guard=1848109190; GUANXI_IDP_guanxi-localhost-idp_GUANXI_IDP_PROFILE_SHIBBOLETH=-3f3920b3--128721807a7---7f9
int cookiesCallback(void* data, const char* key, const char* value) {
	// When we're called, cookieNameOrValue will hold the name of the cookie to look for
	char* cookieNameOrValue = (char*)data;

	// Is this our cookie?
	if (strcmp(cookieNameOrValue, key) == 0) {
		// Return the cookie value. Buffer should be big enough to hold the value
		strcpy(cookieNameOrValue, key);
		return FALSE;
	}

	fprintf(stderr, "callback[%s]: %s %s\n", cookieNameOrValue, key, value);
	fflush(stderr);
	return TRUE;
}

static int mod_guanxi_method_handler(request_rec* r)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(r->server->module_config, &mod_guanxi_module);

	// ////////////////////////////////////////////////////////////////////////////////
	// Guard Services
	// ////////////////////////////////////////////////////////////////////////////////
	const char* service = Util::getServiceName(r->uri);
	if (service != NULL) {
		fprintf(stderr, "SERVICE = %s\n", service);
		fflush(stderr);
		fprintf(stderr, "ARGS = %s\n", r->args);
		fflush(stderr);

		// Verifier service
		if (strcmp(service, gxConfig->verifierURL) == 0) {
			r->content_type = "text/plain";

			const char* sessionID = Util::getRequestParam((const char*)"sessionid", (const char*)r->args);
			if (sessionID != NULL) {
				if (gxConfig->podManager->hasPod(sessionID)) {
					ap_rputs(ENGINE_SESSION_VERIFIED, r);
				}
				else {
					ap_rputs(ENGINE_SESSION_NOTVERIFIED, r);
				}
			}
			else {
				ap_rputs(ENGINE_SESSION_NOTVERIFIED, r);
			}

			return OK;
		}

		// Attribute Consumer Service
		if (strcmp(service, gxConfig->attributeConsumerURL) == 0) {
			r->content_type = "text/plain";
			ap_rputs("OK", r);
			return OK;
		}

		// Podder Service
		if (strcmp(service, gxConfig->podderURL) == 0) {
			const char* sessionID = Util::getRequestParam((const char*)"id", (const char*)r->args);
			Pod* pod = gxConfig->podManager->getPod(sessionID);
			apr_table_set(r->err_headers_out, "Location", pod->getURI().c_str());
			return HTTP_TEMPORARY_REDIRECT;
		}
	} // if (service != NULL)
	// ////////////////////////////////////////////////////////////////////////////////

	// Not a Guard service so we've either finished the SAML round trip or we need to start one

	// Look for our cookie
	char cookieNameOrValue[COOKIE_NAME_OR_VALUE_BUFFER_SIZE];
	strcpy(cookieNameOrValue, gxConfig->cookiePrefix);
	strcat(cookieNameOrValue, gxConfig->entityID);
	apr_table_do(cookiesCallback, (void*)cookieNameOrValue, r->headers_in, NULL);

	/* If we didn't find the cookie or there's no pod for the cookie
	 * we need to start a new session with the Engine
	 */
	if ((strtok(cookieNameOrValue, gxConfig->cookiePrefix) == NULL) ||
			(!gxConfig->podManager->hasPod(cookieNameOrValue))) {
		// Create a new session
		string sessionID = Util::generateUUID();

		// Redirect to the Engine's GPS service
		char gpsService[255];
		strcpy(gpsService, gxConfig->engineGPSURL);
		strcat(gpsService, "?guardid=");
		strcat(gpsService, gxConfig->entityID);
		strcat(gpsService, "&sessionid=");
		strcat(gpsService, sessionID.c_str());
		apr_table_set(r->headers_out, "Location", gpsService);

		// Create a new Pod
		Pod* pod = gxConfig->podManager->addPod(sessionID);
		pod->setURI(string(r->uri));

		/* Create the new cookie tied to the new Pod. Need to use err_headers_out
		 * as headers_out only gets sent on 2XX responses.
		 */
		char* cookie = apr_psprintf(r->pool, "%s%s=%s; path=/; domain=%s", gxConfig->cookiePrefix, gxConfig->entityID, sessionID.c_str(), "sgarbh.smo.uhi.ac.uk");
		apr_table_add(r->err_headers_out, "Set-Cookie", cookie);

		// Redirecting to the Engine's GPS service
		return HTTP_TEMPORARY_REDIRECT;
	}

	// We now have a cookie with a Pod

	return OK;
}





/*
authz_guanxi_check_user_id
authz_guanxi_auth_checker
authz_guanxi_check_user_id
authz_guanxi_auth_checker
*/


static authn_status authn_guanxi_check_password(request_rec *r, const char *user, const char *password) {
	return AUTH_GRANTED;
}

static int authz_guanxi_auth_checker(request_rec *r) {
	fprintf(stderr, "authz_guanxi_auth_checker\n");
	fprintf(stderr, "ap_auth_type = %s\n", r->ap_auth_type);
	fflush(stderr);

	return OK;
}

// redirect here
// assertions here
static int authz_guanxi_check_user_id(request_rec *r) {
	fprintf(stderr, "authz_guanxi_check_user_id\n");
	fflush(stderr);

	r->ap_auth_type = "guanxi";
	r->user = "testuser";
	apr_table_set(r->headers_in, "REMOTE_USER", "testuser");


	const apr_array_header_t *reqs_arr = ap_requires(r);
	require_line* reqs = (require_line*)reqs_arr->elts;
	for (int x=0; x<reqs_arr->nelts; x++) {
		fprintf(stderr, "req, %s\n", reqs[x].requirement);
		fflush(stderr);
	}

	return OK;
}

static int authz_guanxi_access_checker(request_rec *r) {
	fprintf(stderr, "authz_guanxi_access_checker\n");
	fflush(stderr);

	return OK;
}


static const authn_provider authn_guanxi_provider = {
    &authn_guanxi_check_password,
};

static void mod_guanxi_register_hooks(apr_pool_t* p)
{
//  ap_register_provider(p, AUTHN_PROVIDER_GROUP, "guanxi", "0", &authn_guanxi_provider);
	ap_hook_check_user_id(authz_guanxi_check_user_id, NULL, NULL, APR_HOOK_MIDDLE);
  ap_hook_auth_checker(authz_guanxi_auth_checker, NULL, NULL, APR_HOOK_FIRST);
//  ap_hook_access_checker(authz_guanxi_access_checker, NULL, NULL, APR_HOOK_MIDDLE);
//  ap_hook_handler(mod_guanxi_method_handler, NULL, NULL, APR_HOOK_LAST);
}







static void *create_mod_guanxi_config(apr_pool_t* p, server_rec* s)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config *)apr_pcalloc(p, sizeof(mod_guanxi_config));
	gxConfig->podManager = new PodManager();
	gxConfig->attributeConsumerService = new AttributeConsumerService();

	return (void*)gxConfig;
}

// --------------------------------------------------------------------------------------------------------
// Configuration Setters
const char* set_entity_id(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->entityID = apr_pstrdup(parms->pool, arg);
	return NULL;
}

const char* set_cookie_prefix(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->cookiePrefix = apr_pstrdup(parms->pool, arg);
	return NULL;
}

const char* set_verifier_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->verifierURL = apr_pstrdup(parms->pool, arg);
	return NULL;
}

const char* set_attribute_consumer_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->attributeConsumerURL = apr_pstrdup(parms->pool, arg);
	return NULL;
}

const char* set_podder_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->podderURL = apr_pstrdup(parms->pool, arg);
	return NULL;
}

const char* set_engine_gps_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->engineGPSURL = apr_pstrdup(parms->pool, arg);
	return NULL;
}
// --------------------------------------------------------------------------------------------------------

typedef const char* (*config_fn_t)(void);
static const command_rec mod_guanxi_cmds[] =
{
	{
		"GuaxniGuardEntityID", (config_fn_t)set_entity_id, NULL, RSRC_CONF, TAKE1,
		"GuaxniGuardEntityID <string> -- the entityID of the Guanxi Guard",
	},
	{
		"GuaxniGuardCookiePrefix", (config_fn_t)set_cookie_prefix, NULL, RSRC_CONF, TAKE1,
		"GuaxniGuardCookiePrefix <string> -- the prefix for the cookie to use with the Guanxi Guard",
	},
	{
		"GuanxiGuardVerifierURL", (config_fn_t)set_verifier_url, NULL, RSRC_CONF, TAKE1,
		"GuanxiGuardVerifierURL <string> -- the URL of the Guanxi Guard Verifier Service",
	},
	{
		"GuanxiGuardAttributeConsumerURL", (config_fn_t)set_attribute_consumer_url, NULL, RSRC_CONF, TAKE1,
		"GuanxiGuardAttributeConsumerURL <string> -- the URL of the Guanxi Guard Attribute Consumer Service",
	},
	{
		"GuaxniGuardPodderURL", (config_fn_t)set_podder_url, NULL, RSRC_CONF, TAKE1,
		"GuaxniGuardPodderURL <string> -- the URL of the Guanxi Guard Podder Service",
	},
	{
		"GuaxniEngineGPSURL", (config_fn_t)set_engine_gps_url, NULL, RSRC_CONF, TAKE1,
		"GuaxniEngineGPSURL <string> -- the URL of the Guanxi Engine GPS Service",
	},
	{NULL}
};

module AP_MODULE_DECLARE_DATA mod_guanxi_module =
{
	STANDARD20_MODULE_STUFF,
	NULL, // create_dir_config
	NULL, // merge_dir_config
	create_mod_guanxi_config, // create_server_config
	NULL, // merge_server_config
	mod_guanxi_cmds, // cmds
	mod_guanxi_register_hooks // register_hooks
};
