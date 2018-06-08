#include <syslog.h>
#include "common/log.h"

const char* DEFAULT_LOGGING_CONF_FILE = "/etc/jhtm_log_conf";

int initlog(const char* conf_name, const char* app_name){
    int rc = -1;

    if (!conf_name || conf_name[0] == '\0') {
        rc = dzlog_init(DEFAULT_LOGGING_CONF_FILE, app_name);
    }
    else {
        rc = dzlog_init(conf_name, app_name);
    }

    if (rc) {
        zlog_fini();

        openlog(app_name, LOG_CONS | LOG_PID, LOG_USER);
        syslog(LOG_ERR, "Application[%s] failed to init logger!\n", app_name);
        closelog();
        return -1;
    }
    return 0;
}

void releaseLog() {
    zlog_fini();
}
