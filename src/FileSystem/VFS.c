#include <FileSystem.h>
#include <Common.h>
#include <string.h>


static struct Mount *mounts[256] = { NULL };


static inline int isalnum(char ch)
{
	return (ch >= 'A' && ch <= 'Z') ||
	       (ch >= 'a' && ch <= 'z') ||
	       (ch >= '0' && ch <= '9');
}

int FSPrefixValid(const char *prefix)
{
	size_t len = strlen(prefix);

	if(len < 2) return 0;

	for(size_t i = 0; i < len; i++) {
		if(prefix[i] == ':')
			return prefix[i + 1] == ':';

		if(!isalnum(prefix[i]))
			return 0;
	}

	return 0;
}

int FSPathValid(const char *path)
{
	size_t len = strlen(path);

	if(len < 3) return 0;
	if(!FSPrefixValid(path)) return 0;

	for(size_t i = 0; i < len; i++) {
		if(path[i] == ':') {
			path = &path[i + 2];
			break;
		}
	}

	if(path[0] != '/') return 0;

	for(size_t i = 1; i < len; i++) {
		char ch = path[i];
		char ls = i != 0      ? path[i - 1] : 'X';
		char rs = i < len - 1 ? path[i + 1] : 'X';

		if(ch == '/' && (ls == '/' || rs == '/'))
			return 0;
	}

	return 1;
}

size_t FSPrefixExtract(const char *path, char *prefix, size_t max)
{
	size_t len = strlen(path);

	size_t nlen = min(len, max);

	for(size_t i = 0; i < len; i++) {
		if(path[i] == ':') return i;

		if(i < nlen)
			prefix[i] = path[i];
	}

	return 0;
}

const char *FSPrefixSkip(const char *path)
{
	for(size_t i = 0; path[i]; i++)
		if(path[i] == ':')
			return &path[i + 2];

	return NULL;
}

const char *FSPathIterate(const char *cur)
{
	while(*cur != '/' && *cur != '\0')
		cur++;

	if(*cur == '\0') return NULL;
	if(cur[1] == '\0') return NULL;

	return &cur[1];
}

int Mount(struct Mount *mnt)
{
	return 0;
}
