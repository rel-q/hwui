#include "log.h"
#include <stdio.h>


#define LOG_BUF_SIZE 1024

void printLog(int prio, const char* tag,
              const char* fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    printf("LOG: %d, [%s]:%s \n", prio, tag, buf);

}