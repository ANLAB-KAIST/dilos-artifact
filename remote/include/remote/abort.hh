#pragma once
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
void abort(const char *fmt...) {
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
    abort();
}