#include "stdio_impl.h"
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>

/* Used by code compiled on Linux with -D_FORTIFY_SOURCE */

int __vfprintf_chk(FILE * f, int flag, const char *fmt, va_list ap)
{
    return vfprintf(f, fmt, ap);
}

// from http://www.openwall.com/lists/musl/2015/06/17/1
int __vprintf_chk(int flag, const char *fmt, va_list ap)
{
	return vfprintf(stdout, fmt, ap);
}

int __printf_chk (int flag, const char *fmt, ...)
{
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = vfprintf(stdout, fmt, args);
    va_end(args);
    return ret;
}

int __vsnprintf_chk(char *s, size_t maxlen, int flags, size_t slen,
        const char *format, va_list args)
{
    if (slen < maxlen) {
        abort();
    }
    return vsnprintf(s, maxlen, format, args);
}

int __vasprintf_chk (char **__ptr, int __flag, const char *__fmt,
                     va_list __arg)
{
    return vasprintf(__ptr, __fmt, __arg);
}

int __asprintf_chk (char **ptr, int flag, const char *fmt, ...)
{
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = __vasprintf_chk(ptr, flag, fmt, args);
    va_end(args);
    return ret;
}

int __sprintf_chk(char *s, int flags, size_t slen, const char *format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = vsnprintf(s, slen, format, args);
    if (ret > slen) {
        abort();
    }
    va_end(args);
    return ret;
}

int __snprintf_chk (char * s, size_t n, int flag, size_t slen,
                    const char *format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = __vsnprintf_chk(s, n, flag, slen, format, args);
    va_end(args);
    return ret;
}

int __vsprintf_chk (char *s, int flags, size_t slen, const char *format,
                 va_list args)
{
    int ret;
    ret = vsnprintf(s, slen, format, args);
    if (ret > slen) {
        abort();
    }
    return ret;
}

int __swprintf_chk(wchar_t *s, size_t n, int flag, size_t slen, const wchar_t *format, ...)
{
    if(slen < n) {
        abort();
    }
    va_list args;
    int ret;
    va_start(args, format);
    ret = vswprintf(s, n, format, args);
    va_end(args);
    return ret;
}
