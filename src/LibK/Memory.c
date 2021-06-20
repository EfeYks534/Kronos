#include "string.h"

int memcmp(const void *str1, const void *str2, size_t n)
{
	if(!n) return 0;

	while(--n && *(char*)str1 == *(char*)str2) {
		str1 = ((char*)str1) + 1;
		str2 = ((char*)str2) + 1;
	}

	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	if(!n) return dest;

	char *d = dest;
	char *s = (char*) src;

	while(n--)
		*d++ = *s++;

	return dest;
}

void *memset(void *str, int c, size_t n)
{
	if(!n) return str;

	char *s = str;

	while(n--)
		*s++ = c;

	return str;
}

int strcmp(const char *str1, const char *str2)
{
	while(*str1 && *str1 == *str2) {
		str1++;
		str2++;
	}

	return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
	if(!n) return 0;

	while(n-- && *str1 && *str1 == *str2) {
		str1++;
		str2++;
	}

	return *(unsigned char*)str1 - *(unsigned char*)str2;

}

size_t strlen(const char *str)
{
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}
