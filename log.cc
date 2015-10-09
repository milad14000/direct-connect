#include "log.h"
#include <cstring>
#include <errno.h>
#include <cstdlib>

namespace NSimulator {
Logger::Logger(int level, char* name) {
  level_ = level; 	/* Set log level */
  name_ = name;
  if (name == NULL || !strlen(name)) {
    f_ = stderr;
  } else { 
    f_ = fopen(name, "w");
    if (f_ == NULL) {
      fprintf(stderr, "Opening log file '%s' failed: %s", name,
      		strerror(errno));
      f_ = stderr;
    }
  }
}

Logger::~Logger() {
  if (f_ != stderr) {
    fclose(f_);
  }
}

void
Logger::log(int level, const char *fmt, va_list args) {
  if (level <= level_) { 
    vfprintf(f_, fmt, args);
    fprintf(f_, "\n");
  }
}

void
Logger::warn(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log(LOG_WARN, fmt, args);
  va_end(args);
}

void 
Logger::info(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log(LOG_INFO, fmt, args);
  va_end(args);
}

void
Logger::debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log(LOG_DEBUG, fmt, args);
  va_end(args);
}

void 
Logger::error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log(LOG_ERR, fmt, args);
  va_end(args);
  exit(0);
}
}
