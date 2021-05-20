#pragma once

#include <stddef.h>
#include <stdint.h>

#define GDT_ATTR_PRESENT  (1 << 7)
#define GDT_ATTR_RING3    ((1 << 5) | (1 << 6))
#define GDT_ATTR_CODEDATA (1 << 4)
#define GDT_ATTR_EXEC     (1 << 3)
#define GDT_ATTR_GROWDOWN (1 << 2)
#define GDT_ATTR_RDWR     (1 << 1)
#define GDT_ATTR_TSS      (1 << 0)
