/**
 *     Copyright 2016-2019 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <libcouchbase/couchbase.h>
#include <php.h>

#include "log.h"

static const char *level_to_string(int severity)
{
    switch (severity) {
    case LCB_LOG_TRACE:
        return "TRAC";
    case LCB_LOG_DEBUG:
        return "DEBG";
    case LCB_LOG_INFO:
        return "INFO";
    case LCB_LOG_WARN:
        return "WARN";
    case LCB_LOG_ERROR:
        return "EROR";
    case LCB_LOG_FATAL:
        return "FATL";
    default:
        return "";
    }
}

#define PCBC_LOG_MSG_SIZE 1024

static void log_handler(struct lcb_logprocs_st *procs, unsigned int iid, const char *subsys, int severity,
                        const char *srcfile, int srcline, const char *fmt, va_list ap)
{
    struct pcbc_logger_st *logger = (struct pcbc_logger_st *)procs;
    char buf[PCBC_LOG_MSG_SIZE] = {0};
    TSRMLS_FETCH();

    if (severity < logger->minlevel) {
        return;
    }

    pcbc_log_formatter(buf, PCBC_LOG_MSG_SIZE, level_to_string(severity), subsys, srcline, iid, NULL, 1, fmt, ap);
    php_log_err(buf TSRMLS_CC);
}

struct pcbc_logger_st pcbc_logger = {{0 /* version */, {{log_handler} /* v1 */} /*v*/},
                                     /** Minimum severity */
                                     LCB_LOG_INFO};

void pcbc_log(int severity, lcb_t instance, const char *subsys, const char *srcfile, int srcline, const char *fmt, ...)
{
    va_list ap;
    char buf[PCBC_LOG_MSG_SIZE] = {0};
    TSRMLS_FETCH();

    if (severity < pcbc_logger.minlevel) {
        return;
    }

    va_start(ap, fmt);
    pcbc_log_formatter(buf, PCBC_LOG_MSG_SIZE, level_to_string(severity), subsys, srcline, 0, (void *)instance, 0, fmt,
                       ap);
    va_end(ap);

    php_log_err(buf TSRMLS_CC);
}
