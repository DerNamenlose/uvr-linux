#include <stdarg.h>
#include <stdio.h>
#include "logging.h"
#include <boost/config/posix_features.hpp>

int isDaemon;

void initlog(int daemon)
{
    isDaemon = daemon;
    if (isDaemon) {
        openlog("dlogg-reader", 0, LOG_DAEMON);
    }
}

void log_output ( int priority, char* format, ... )
{
    va_list ap;
    va_start(ap, format);
    if (isDaemon) {
        vsyslog(priority, format, ap);
    }
    else {
        vfprintf(stderr, format, ap);
    }
    va_end(ap);    
}

void endlog()
{
    if (isDaemon) {
        closelog();
    }
}
