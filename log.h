#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <stdarg.h>

#define LOG_DEBUG  4	/* Debugging */
#define LOG_INFO   3    /* Informational */
#define LOG_WARN   2	/* Warning! */ 
#define LOG_ERR	   1	/* Error */

#define MAX_BUF   256   /* max length of log message */

namespace NSimulator {
class Logger {
  protected:
    char *name_;	/* Log file name */
    int level_;		/* Log level */
    FILE *f_;		/* Log file descriptor */

  public:
    Logger(int level, char* name);
    ~Logger();

    void log(int level, const char *fmt, va_list args); 
    void debug(const char *fmt, ...); 
    void info(const char *fmt, ...); 
    void warn(const char *fmt, ...); 
    void error(const char *fmt, ...); 
    void set_level(int level) { level_ = level; };
};


static struct Logger logger(LOG_DEBUG, NULL);
}

#endif
