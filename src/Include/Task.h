#pragma once

#include <stddef.h>
#include <stdint.h>

void Lock(int64_t *lock);

void Unlock(int64_t *lock);
