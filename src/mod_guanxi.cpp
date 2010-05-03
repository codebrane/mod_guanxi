#include "httpd.h"
#include "http_config.h"
#include "apr_strings.h"

#include "util.h"
#include "podmanager.h"

extern "C" module AP_MODULE_DECLARE_DATA mod_guanxi_module;

struct mod_guanxi_config {
	char* cookiePrefix;
	char* verifierURL;
	char* attributeConsumerURL;
	char* podderURL;
	PodManager* podManager;
};

static int cookies_callback(void* data, const char* key, const char* value)
{
	const char* label = (const char*)data;
	fprintf(stderr, "callback[%s]: %s %s\n", label, key, value);
	fflush(stderr);
	return TRUE;
}

static int mod_guanxi_method_handler(request_rec* r)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(r->server->module_config, &mod_guanxi_module);

	const char* cookieValue = Util::getCookieValue(r, gxConfig->cookiePrefix);
	if (cookieValue == NULL) {
		apr_table_setn(r->headers_out, "Location", "http://codebrane.com/blog");
		return HTTP_TEMPORARY_REDIRECT;
	}

	char szMessage[255];
	strcpy(szMessage, "GX:");
	strcat(szMessage, r->the_request);
	strcat(szMessage, "\n");
	strcat(szMessage, r->uri);
	strcat(szMessage, "\n");
	fprintf(stderr, szMessage);
	fflush(stderr);

	fprintf(stderr, "PODS START ----------------------------\n");
	string s = Util::generateUUID();
	fprintf(stderr, s.c_str());
	fflush(stderr);
	gxConfig->podManager->addPod(s);
	Pod* pod = gxConfig->podManager->getPod(s);
	fprintf(stderr, pod->getSessionID().c_str());
	fprintf(stderr, "PODS END ----------------------------\n");
	fflush(stderr);


	apr_table_setn(r->headers_out, "Location", "http://codebrane.com/blog");

	char* cookie = apr_psprintf(r->pool, "%s=%s; path=/", "__GX", "test");
	apr_table_add(r->headers_out, "Set-Cookie", cookie);

	apr_table_do(cookies_callback, (void*)"headers_in", r->headers_in, NULL);
	fprintf(stderr, "headers_in FINISHED\n");
	fprintf(stderr, "----------------------------\n");
	fflush(stderr);
	apr_table_do(cookies_callback, (void*)"headers_out", r->headers_out, NULL);
	fprintf(stderr, "headers_out FINISHED\n");
	fprintf(stderr, "----------------------------\n");
	fflush(stderr);

	fprintf(stderr,"GuanxiGuardVerifierURL = ");
	fprintf(stderr, gxConfig->verifierURL);
	fprintf(stderr,"\n");
	fprintf(stderr,"GuanxiGuardAttributeConsumerURL = ");
	fprintf(stderr, gxConfig->attributeConsumerURL);
	fprintf(stderr,"\n");
	fprintf(stderr,"GuaxniGuardPodderURL = ");
	fprintf(stderr, gxConfig->podderURL);
	fprintf(stderr,"\n");
	fprintf(stderr,"GuaxniGuardCookiePrefix = ");
	fprintf(stderr, gxConfig->cookiePrefix);
	fprintf(stderr,"\n");
	fflush(stderr);

	fprintf(stderr, "RETURNING\n");
	fflush(stderr);

	return HTTP_TEMPORARY_REDIRECT;
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
const char* set_cookie_prefix(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->cookiePrefix = (char *)arg;
	return NULL;
}

const char* set_verifier_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->verifierURL = (char *)arg;
	return NULL;
}

const char* set_attribute_consumer_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->attributeConsumerURL = (char *)arg;
	return NULL;
}

const char* set_podder_url(cmd_parms* parms, void* mconfig, const char* arg)
{
	mod_guanxi_config* gxConfig = (mod_guanxi_config*)ap_get_module_config(parms->server->module_config, &mod_guanxi_module);
	gxConfig->podderURL = (char *)arg;
	return NULL;
}
// --------------------------------------------------------------------------------------------------------

typedef const char* (*config_fn_t)(void);
static const command_rec mod_guanxi_cmds[] =
{
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
