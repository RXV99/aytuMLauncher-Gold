#include "vitajme.h"
#include <psp2/net/http.h>

// -----------------------------------------------------------------------------
// Javacall Network HAL (mapping SceHttp to phoneME)
// -----------------------------------------------------------------------------

typedef unsigned int javacall_result;
typedef int javacall_handle;
#define JAVACALL_OK 0
#define JAVACALL_FAIL 1

static int g_http_tmpl = -1;

javacall_result javacall_network_init(void) {
    sceHttpInit(1024 * 1024);
    g_http_tmpl = sceHttpCreateTemplate("VitaJME/1.0", SCE_HTTP_VERSION_1_1, SCE_TRUE);
    return (g_http_tmpl < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_network_open(const char *url, javacall_handle *conn_handle) {
    if (g_http_tmpl < 0 || !url || !conn_handle) return JAVACALL_FAIL;
    
    *conn_handle = sceHttpCreateConnectionWithURL(g_http_tmpl, url, SCE_TRUE);
    return (*conn_handle < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_network_close(javacall_handle conn_handle) {
    sceHttpDeleteConnection(conn_handle);
    return JAVACALL_OK;
}

// ... Additional implementations for send/recv would go here ...
