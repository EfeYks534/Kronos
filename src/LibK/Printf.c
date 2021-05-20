#include <Common.h>
#include "stdio.h"


static char HexChar(uint64_t tmp)
{
	return tmp > 9 ? 'A' + tmp - 10 : '0' + tmp;
}

#define putchar(x) { if(!flag) { if(i >= len) {flag = 1;} else {str[k++] = x;} } else { k++; } }

int vsnprintf(char *str, int len, const char *fmt, va_list ap)
{
	int i = 0;
	int k = 0;
	uint8_t flag = 0;

	while(1) {
		char ch = fmt[i++];
		if(ch == 0) break;

		if(ch != '%') {
			putchar(ch);
		} else {
			ch = fmt[i++];
			switch(ch)
			{
			case '%':
				putchar('%');
				break;
			case 'x': {
				ch = fmt[i++];
				uint64_t tmp = 0;
				int64_t j = 0;
				switch(ch)
				{
				case 'b':
					tmp = va_arg(ap, int32_t);
					j = 1;
					break;
				case 's':
					tmp = va_arg(ap, int32_t);
					j = 3;
					break;
				case 'i':
					tmp = va_arg(ap, int32_t);
					j = 7;
					break;
				case 'l':
					tmp = va_arg(ap, int64_t);
					j = 15;
					break;
				default:
					tmp = va_arg(ap, int32_t);
					j = 1;
					break;
				}


				for(; j >= 0; j--) {
					putchar( HexChar( (tmp >> (j << 2)) & 0x0F ) );
				}

				break;
			  }
			case 'b': {
				ch = fmt[i++];
				uint64_t tmp = 0;
				int64_t j = 0;
				switch(ch)
				{
				case 'b':
					tmp = va_arg(ap, int32_t);
					j = 7;
					break;
				case 's':
					tmp = va_arg(ap, int32_t);
					j = 15;
					break;
				case 'i':
					tmp = va_arg(ap, int32_t);
					j = 31;
					break;
				case 'l':
					tmp = va_arg(ap, int64_t);
					j = 63;
					break;
				default:
					tmp = va_arg(ap, int32_t);
					j = 7;
					break;
				}

				for(; j >= 0; j--) {
					putchar( ( (tmp >> j) & 1 ) + '0' );
					if(j % 4 == 0 && j != 0) putchar('_');
				}

				break;
			  }
			case 'd': {
				int32_t tmp = va_arg(ap, int32_t);

				if(tmp == 0)
					putchar('0');

				if(tmp < 0) {
					putchar('-');
					tmp = -tmp;
				}

				char buf[20] = { 0 };
				int j = 0;

				while(tmp != 0) {
					int rem = tmp % 10;
					buf[j++] = rem + '0';
					tmp = tmp / 10;
				}

				j--;
				while(j >= 0)
					putchar(buf[j--]);

				break;
			  }
			case 'l': {
				int64_t tmp = va_arg(ap, int64_t);

				if(tmp == 0)
					putchar('0');

				if(tmp < 0) {
					putchar('-');
					tmp = -tmp;
				}

				char buf[20] = { 0 };
				int j = 0;

				while(tmp != 0) {
					int rem = tmp % 10;
					buf[j++] = rem + '0';
					tmp = tmp / 10;
				}

				j--;
				while(j >= 0)
					putchar(buf[j--]);

				break;
			  }
			case 's': {
				char *str1 = va_arg(ap, char*);

				int j = 0;
				while(str1[j] != 0)
					putchar(str1[j++]);
				break;
			  }
			case 'c': {
				char c = (char) va_arg(ap, int);
				putchar(c);
				break;
			  }
			}
		}
	}

	return k;
}

int vsprintf(char *str, const char *fmt, va_list ap)
{
	return vsnprintf(str, 65536, fmt, ap);
}

int sprintf(char *str, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	int len = vsprintf(str, fmt, ap);

	va_end(ap);

	return len;
}

int snprintf(char *str, int size, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(str, size, fmt, ap);

	va_end(ap);

	return len;
}
