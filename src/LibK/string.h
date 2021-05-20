#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef NULL
#define NULL ((void*) 0)
#endif

int memcmp(const void *str1, const void *str2, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *str, int c, size_t n);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t n);

size_t strlen(const char *str);
