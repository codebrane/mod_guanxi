#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"

#include "apr_strings.h"

#include "util.h"
#include "podmanager.h"

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
};

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

	// Look for our cookie
	char cookieNameOrValue[COOKIE_NAME_OR_VALUE_BUFFER_SIZE];
	strcpy(cookieNameOrValue, gxConfig->cookiePrefix);
	apr_table_do(cookiesCallback, (void*)cookieNameOrValue, r->headers_in, NULL);

	const char* service = Util::getServiceName(r->uri);
	if (service != NULL) {
		fprintf(stderr, "SERVICE = %s\n", service);
		fflush(stderr);
		fprintf(stderr, "ARGS = %s\n", r->args);
		fflush(stderr);

		// Verifier service
		if (strcmp(service, gxConfig->verifierURL) == 0) {
			r->content_type = "text/plain";

			const char* sessionid = Util::getRequestParam((const char*)"sessionid", (const char*)r->args);
			if (sessionid != NULL) {
				if (gxConfig->podManager->getPod(sessionid) != NULL) {
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
		} // if (strcmp(service, gxConfig->verifierURL) == 0)
	} // if (service != NULL)


	// If we didn't find the cookie we need to start a new session with the Engine
	if (strcmp(cookieNameOrValue, gxConfig->cookiePrefix) == 0) {
		// Create a new Pod for the session
		string sessionid = Util::generateUUID();
		gxConfig->podManager->addPod(sessionid);

		// Redirect to the Engine's GPS service
		char gpsService[255];
		strcpy(gpsService, gxConfig->engineGPSURL);
		strcat(gpsService, "?guardid=");
		strcat(gpsService, gxConfig->entityID);
		strcat(gpsService, "&sessionid=");
		strcat(gpsService, sessionid.c_str());
		apr_table_set(r->headers_out, "Location", gpsService);

		return HTTP_TEMPORARY_REDIRECT;
	}


	//char* cookie = apr_psprintf(r->pool, "%s=%s; path=/", "__GX", "test");
	//apr_table_add(r->headers_out, "Set-Cookie", cookie);

	return OK;
}

static void mod_guanxi_register_hooks(apr_pool_t* p)
{
	ap_hook_handler(mod_guanxi_method_handler, NULL, NULL, APR_HOOK_LAST);
}

static void *create_mod_guanxi_config(apr_pool_t* p, server_rec* s)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config *)apr_pcalloc(p, sizeof(mod_guanxi_config));
	gxConfig->podManager = new PodManager();

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
