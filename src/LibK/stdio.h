#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

int vsnprintf(char *str, int size, const char *fmt, va_list ap);

int vsprintf(char *str, const char *fmt, va_list ap);

int sprintf(char *str, const char *fmt, ...);

int snprintf(char *str, int size, const char *fmt, ...);
