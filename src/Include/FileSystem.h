#pragma once

#include <Device.h>
#include <stddef.h>
#include <stdint.h>


struct File;

struct Mount
{
	char        prefix[64];
	size_t            type;
	void         *internal;
	struct DevStorage *dev;

	struct File *(*open)(struct Mount *mnt, const char *path);
};

struct File
{
	struct Mount *mnt;

	void  (*read) (struct File *f, void*, size_t, size_t);
	void (*write) (struct File *f, void*, size_t, size_t);
	void (*close) (struct File *f);

	uint8_t internal[256];

	size_t path_len;
	char     path[];
};


int FSPathValid(const char *path);

int FSPrefixValid(const char *prefix);

size_t FSPrefixExtract(const char *path, char *prefix, size_t max);

const char *FSPrefixSkip(const char *path);

const char *FSPathIterate(const char *cur);

int Mount(struct Mount *mnt);
