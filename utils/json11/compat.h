#pragma once

#ifdef _MSC_VER

#define NOEXCEPT 

#if _MSC_VER <= 1800 // VS 2013
	#define noexcept
	#define snprintf _snprintf_s
#endif

#else

#define NOEXCEPT noexcept

#endif // _MSC_VER

#if 0

#define NOEXCEPT

#include <stdarg.h>

inline int c99_vsnprintf(char* str, size_t size, char* format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char* str, size_t size, char* format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(str, size, format, ap);
	va_end(ap);

	return count;
}

#define snprintf c99_snprintf

#endif
