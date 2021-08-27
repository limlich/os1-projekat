#include <DOS.H>
#include <STDIO.H>
#include <STDARG.H>

#include "lock.h"

int syncPrintf(const char *format, ...)
{
	int res;
	va_list args;
	SOFT_LOCKED(
		va_start(args, format);
	res = vprintf(format, args);
	va_end(args);
	);
	return res;
}
