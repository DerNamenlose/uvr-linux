#include <stdarg.h>
#include <stdio.h>
#include "logging.h"

int isDaemon;
int debug = 0;

void enable_debug()
{
    debug = 1;
}

void initlog(int daemon)
{
    isDaemon = daemon;
    if (isDaemon) {
        openlog("dlogg-reader", 0, LOG_DAEMON);
    }
}

void log_output ( int priority, char* format, ... )
{
    if (!debug && priority == LOG_DEBUG) {
        return; // switch of debugging output if not needed
    }
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
