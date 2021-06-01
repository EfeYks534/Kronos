#include <Common.h>
#include <Task.h>
#include <Device.h>
#include <Stivale2.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define BLACK    0x000000
#define RED      0xAA0000
#define GREEN    0x00AA00
#define YELLOW   0xAA5500
#define BLUE     0x0000AA
#define MAGENTA  0xAA00AA
#define CYAN     0x00AAAA
#define WHITE    0xAAAAAA
#define BBLACK   0x555555
#define BRED     0xEE5555
#define BGREEN   0x55EE55
#define BYELLOW  0xEEEE55
#define BBLUE    0x5555EE
#define BMAGENTA 0xEE55EE
#define BCYAN    0x55EEEE
#define BWHITE   0xEEEEEE

static uint8_t term_font[4096] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x81, 0xa5, 0x81, 0x81, 0xbd, 0x99, 0x81, 0x81, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0xff, 0xdb, 0xff, 0xff, 0xc3, 0xe7, 0xff, 0xff, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x3e, 0x7f, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x7e, 0x3c, 0xc3, 0xe7, 0xe7, 0xdb, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x08, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3e, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xc1, 0xc1, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x76, 0x62, 0x62, 0x76, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x89, 0x9d, 0x9d, 0x89, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x3f, 0x0f, 0x1d, 0x38, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x1c, 0x7f, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0c, 0x0e, 0x0d, 0x0d, 0x0e, 0x0d, 0x0d, 0x0c, 0x1c, 0x7f, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0x1f, 0x3b, 0x33, 0x37, 0x3b, 0x33, 0x33, 0x37, 0x75, 0x57, 0x70, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x18, 0x18, 0xdb, 0x3c, 0xe7, 0x3c, 0xdb, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x60, 0x70, 0x78, 0x7c, 0x7e, 0x7f, 0x7e, 0x7c, 0x78, 0x70, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x3e, 0x7f, 0x1c, 0x1c, 0x1c, 0x7f, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3f, 0x6d, 0x6d, 0x6d, 0x3d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x60, 0x3e, 0x63, 0x63, 0x3e, 0x03, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x3e, 0x7f, 0x1c, 0x1c, 0x1c, 0x7f, 0x3e, 0x1c, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x3e, 0x7f, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x7f, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x38, 0x1c, 0x0e, 0x7f, 0x7f, 0x0e, 0x1c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0e, 0x1c, 0x38, 0x7f, 0x7f, 0x38, 0x1c, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x36, 0x7f, 0x36, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x1c, 0x3e, 0x3e, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x3e, 0x3e, 0x1c, 0x1c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x33, 0x77, 0xee, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0xff, 0xff, 0x66, 0x66, 0xff, 0xff, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x3c, 0x66, 0x30, 0x18, 0x0c, 0x66, 0x3c, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x51, 0x73, 0x06, 0x0c, 0x18, 0x30, 0x67, 0x45, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0x66, 0x3c, 0x38, 0x6b, 0x66, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0e, 0x1c, 0x38, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x30, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x30, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x0c, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x0c, 0x38, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x63, 0x36, 0x1c, 0x7f, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0c, 0x0c, 0x18, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x36, 0x63, 0x63, 0x6b, 0x6b, 0x63, 0x63, 0x36, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0e, 0x1e, 0x36, 0x66, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x03, 0x03, 0x03, 0x3e, 0x03, 0x03, 0x03, 0x03, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0x0f, 0x1b, 0x33, 0x63, 0x63, 0x7f, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x60, 0x60, 0x60, 0x7e, 0x03, 0x03, 0x03, 0x03, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0x30, 0x60, 0x60, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x63, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x36, 0x1c, 0x36, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x3f, 0x03, 0x03, 0x03, 0x06, 0x7c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x18, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x06, 0x0c, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x6f, 0x6b, 0x6b, 0x6e, 0x60, 0x60, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x36, 0x63, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x63, 0x63, 0x63, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1e, 0x33, 0x61, 0x60, 0x60, 0x60, 0x60, 0x61, 0x33, 0x1e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7c, 0x66, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x66, 0x7c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x60, 0x60, 0x60, 0x7e, 0x60, 0x60, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x60, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1e, 0x33, 0x63, 0x60, 0x60, 0x67, 0x63, 0x63, 0x33, 0x1e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x78, 0x6c, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x77, 0x7f, 0x6b, 0x6b, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x73, 0x6b, 0x67, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7b, 0x6f, 0x3e, 0x06, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x7e, 0x63, 0x63, 0x63, 0x7e, 0x6c, 0x66, 0x66, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x30, 0x18, 0x0c, 0x06, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36, 0x1c, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x6b, 0x6b, 0x7f, 0x77, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x22, 0x36, 0x1c, 0x1c, 0x36, 0x22, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x03, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 
	0x00, 0x70, 0x38, 0x1c, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x60, 0x60, 0x6e, 0x73, 0x63, 0x63, 0x63, 0x73, 0x6e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x60, 0x60, 0x60, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x03, 0x03, 0x3b, 0x67, 0x63, 0x63, 0x63, 0x67, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1e, 0x33, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x67, 0x63, 0x63, 0x63, 0x67, 0x3b, 0x03, 0x03, 0x7e, 0x00, 
	0x00, 0x00, 0x60, 0x60, 0x60, 0x6e, 0x73, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c, 0x00, 
	0x00, 0x00, 0x60, 0x60, 0x60, 0x66, 0x7c, 0x70, 0x78, 0x7c, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x7f, 0x6b, 0x6b, 0x6b, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x76, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x73, 0x63, 0x63, 0x63, 0x73, 0x6e, 0x60, 0x60, 0x60, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x67, 0x63, 0x63, 0x63, 0x67, 0x3b, 0x03, 0x03, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x3b, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x66, 0x30, 0x18, 0x0c, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x6b, 0x6b, 0x6b, 0x7f, 0x36, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x36, 0x1c, 0x1c, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x67, 0x3b, 0x03, 0x03, 0x7e, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0e, 0x18, 0x18, 0x18, 0x70, 0x18, 0x18, 0x18, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x18, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x18, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x36, 0x63, 0x63, 0x63, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1e, 0x33, 0x61, 0x60, 0x60, 0x60, 0x60, 0x61, 0x33, 0x1e, 0x06, 0x06, 0x7c, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0x0c, 0x18, 0x00, 0x3e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x08, 0x1c, 0x36, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x36, 0x36, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x0c, 0x06, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x1c, 0x36, 0x1c, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x60, 0x60, 0x60, 0x63, 0x3e, 0x06, 0x06, 0x3c, 0x00, 
	0x00, 0x08, 0x1c, 0x36, 0x00, 0x3e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x36, 0x36, 0x00, 0x3e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x0c, 0x06, 0x00, 0x3e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x6c, 0x6c, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x3c, 0x66, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x30, 0x18, 0x0c, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x63, 0x00, 0x1c, 0x36, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x1c, 0x36, 0x1c, 0x36, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0e, 0x18, 0x7f, 0x60, 0x60, 0x60, 0x7e, 0x60, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x3b, 0x1b, 0x3f, 0x6c, 0x6c, 0x37, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0x3c, 0x6c, 0x6c, 0x7f, 0x6c, 0x6c, 0x6c, 0x6c, 0x6f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x08, 0x1c, 0x36, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x36, 0x36, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x0c, 0x06, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x3c, 0x66, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x30, 0x18, 0x0c, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x36, 0x36, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x67, 0x3b, 0x03, 0x03, 0x7e, 0x00, 
	0x00, 0x63, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x63, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x3c, 0x66, 0x62, 0x60, 0x62, 0x66, 0x3c, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x3e, 0x32, 0x30, 0x30, 0x7e, 0x30, 0x30, 0x33, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x6c, 0x76, 0x66, 0x66, 0x78, 0x64, 0x64, 0x6e, 0x64, 0x64, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0e, 0x19, 0x18, 0x18, 0x18, 0x7f, 0x0c, 0x0c, 0x0c, 0x4c, 0x38, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0x0c, 0x18, 0x00, 0x3c, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0x0c, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0x0c, 0x18, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0c, 0x18, 0x30, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3b, 0x6e, 0x00, 0x6c, 0x76, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x73, 0x8c, 0x63, 0x63, 0x63, 0x73, 0x6b, 0x67, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3e, 0x66, 0x66, 0x3f, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3c, 0x66, 0x66, 0x3c, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x30, 0x73, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x70, 0x70, 0x70, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x60, 0x60, 0x61, 0x63, 0x66, 0x0c, 0x18, 0x30, 0x6e, 0x43, 0x06, 0x0c, 0x1f, 0x00, 0x00, 
	0x00, 0x60, 0x60, 0x61, 0x63, 0x66, 0x0c, 0x18, 0x33, 0x67, 0x4b, 0x13, 0x3f, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x66, 0x33, 0x66, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 
	0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 
	0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0xf6, 0x06, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x06, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0xf6, 0x06, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x30, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0xf7, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xf7, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0xf7, 0x00, 0xf7, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xff, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0xce, 0xcc, 0xcc, 0xcc, 0xce, 0x7b, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x2c, 0x76, 0x66, 0x66, 0x6e, 0x63, 0x63, 0x63, 0x63, 0x6e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x63, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xb6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0x63, 0x30, 0x18, 0x0c, 0x0c, 0x18, 0x30, 0x63, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7b, 0x60, 0x60, 0x60, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x8c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x7e, 0xdb, 0xdb, 0xdb, 0xdb, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xbd, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x66, 0xe7, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x73, 0x38, 0x1c, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xdb, 0xdb, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x7e, 0xdb, 0xdb, 0xdb, 0x7e, 0x18, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x30, 0x60, 0x3c, 0x60, 0x30, 0x1e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x70, 0x38, 0x1c, 0x0e, 0x1c, 0x38, 0x70, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0e, 0x1c, 0x38, 0x70, 0x38, 0x1c, 0x0e, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0e, 0x1b, 0x1b, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xd8, 0xd8, 0xd8, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x7e, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x6e, 0x00, 0x3b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3c, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x6c, 0x6c, 0x3c, 0x1c, 0x00, 0x00, 0x00, 
	0x00, 0x6c, 0x76, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3c, 0x26, 0x0c, 0x18, 0x30, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct TermState
{
	uint32_t  color;
	size_t    width;
	size_t   height;
};

static inline void TermPlot(struct DevTerminal *term, uint64_t x, uint64_t y, uint32_t c)
{
	struct TermState *state = term->state;

	if(x >= state->width || y >= state->height) return;

	term->fb[x + y * state->width] = c;
}

static void TermPut(struct DevTerminal *term, char ch)
{
	struct TermState *state = term->state;

	uint32_t *fb = term->fb;

	size_t ox = term->column * 8  + 1;
	size_t oy = term->line   * 16 + 1;

	for(size_t y = 0; y < 16; y++) {
		uint8_t data = term_font[ch * 16 + y];

		for(size_t x = 0; x < 8; x++) {
			if((data >> x) & 1) {
				size_t px = ox + 8 - x;
				size_t py = oy + y;

				TermPlot(term, px, py, state->color);
			}
		}
	}
}

static int atoi(const char *str)
{
	int num = 0;

	int i = 0;

	while(str[i]) {
		char ch = str[i++];

		if(!(ch >= '0' && ch <= '9')) return 0;

		num = num * 10 + (ch - '0');
	}

	return num;
}

static void SrDebug(const char *fmt, ...)
{
static char log_buffer[4096] = { 0 };

    va_list ap; 

    va_start(ap, fmt);

    int len = vsnprintf(log_buffer, 4095, fmt, ap);

    va_end(ap);

	struct DevSerial *sr;
	sr = DeviceGet(DEV_CATEGORY_DATA, DEV_TYPE_DTSERIAL, "Serial(COM1)");

	if(sr != NULL)
		sr->write(sr, log_buffer, len);
}

static void TermWrite(struct DevTerminal *term, const char *str)
{
	struct TermState *state = term->state;

	Lock(&term->dev.lock);

	if(!term->dev.enabled) {
		Unlock(&term->dev.lock);
		return;
	}

	size_t i = 0;

	while(str[i]) {
		char ch = str[i++];

		switch(ch)
		{
		case '\x1B': {
			ch = str[i++];
			if(ch != '[')
				return;

			ch = str[i++];

			int attr[10] = { 0 };
			int count   = 0;

			while(ch != 'm' && count < 10) {
				char buf[4] = { 0 };

				for(int j = 0; j < 2 && (ch >= '0' && ch <= '9'); j++) {
					buf[j] = ch;
					ch = str[i++];
				}

				attr[count++] = atoi(buf);

				if(ch != ';' && ch != 'm') {
					SrDebug("Got '%d', expected ';' or 'm'\n", ch);
					return;
				}

				if(ch == ';')
					ch = str[i++];
			}

			if(ch != 'm') {
				SrDebug("Got '%d', expected 'm'\n", ch);
				return;
			}

			for(int i = 0; i < count; i++) {
				switch(attr[i])
				{
				case  0: state->color = BWHITE;   break;
				case 30: state->color = BLACK;    break;
				case 31: state->color = RED;      break;
				case 32: state->color = GREEN;    break;
				case 33: state->color = YELLOW;   break;
				case 34: state->color = BLUE;     break;
				case 35: state->color = MAGENTA;  break;
				case 36: state->color = CYAN;     break;
				case 37: state->color = WHITE;    break;
				case 90: state->color = BBLACK;   break;
				case 91: state->color = BRED;     break;
				case 92: state->color = BGREEN;   break;
				case 93: state->color = BYELLOW;  break;
				case 94: state->color = BBLUE;    break;
				case 95: state->color = BMAGENTA; break;
				case 96: state->color = BCYAN;    break;
				case 97: state->color = BWHITE;   break;
				default: break;
				}
			}

			break;
		  }
		case '\v':
			term->line+=4;
			break;
		case '\f':
			term->line++;
			break;
		case '\r':
			term->column = 0;
			break;
		case '\n':
			term->line++;
			term->column = 0;
			break;
		case '\t':
			TermWrite(term, "    ");
			break;
		default:
			TermPut(term, ch);
			term->column++;
			break;
		}

		if(term->line >= state->height / 16) {
			term->line--;

			for(size_t y = 0; y < state->height - 16; y++) {
				for(size_t x = 0; x < state->width; x++) {
					term->fb[x + y * state->width] = term->fb[x + (y+16) * state->width];
				}
			}

			for(size_t y = state->height - 16; y < state->height; y++) {
				for(size_t x = 0; x < state->width; x++) {
					term->fb[x + y * state->width] = 0;
				}
			}
		}

		if(term->column >= state->width / 8) {
			term->line++;
			term->column = 0;
		}
	}

	Unlock(&term->dev.lock);
}



static struct TermState tstate = { BWHITE };

static struct DevTerminal term = { 0 };

static void KEINIT TermInit()
{
	struct stivale2_struct_tag_framebuffer *fb;
	fb = Stivale2GetTag(STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

	if(fb == NULL) return;

	if(fb->framebuffer_pitch != 4096 || fb->framebuffer_bpp != 32) return;

	tstate.width  = fb->framebuffer_width;
	tstate.height = fb->framebuffer_height;

	memcpy(term.dev.name, "Terminal", strlen("Terminal"));

	term.dev.type    = DEV_TYPE_GRTERM;
	term.dev.enabled = 1;
	term.dev.lock    = 0;
	term.state       = &tstate;
	term.fb          = (uint32_t*) fb->framebuffer_addr;
	term.write       = TermWrite;

	DeviceRegister(DEV_CATEGORY_TERM, &term.dev);

	DevicePrimarySet(DEV_CATEGORY_TERM, &term.dev);
}