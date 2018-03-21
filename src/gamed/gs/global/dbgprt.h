#ifndef GAMED_GS_GLOBAL_DBGPRT_H_
#define GAMED_GS_GLOBAL_DBGPRT_H_

#include <stdio.h>

namespace gamed {

extern bool __PRINT_FLAG;

inline void __SETPRTFLAG(bool flag)
{
	__PRINT_FLAG  = flag;
}

#ifdef __NO_STD_OUTPUT__
inline int __PRINTF(const char * fmt, ...)
{
}
#else
#include <stdarg.h>
inline int __PRINTF(const char * fmt, ...)
#ifdef __GNUC__	
		__attribute__ ((format (printf, 1, 2)))
#endif
;

inline int __PRINTF(const char * fmt, ...)
{
	if (__builtin_expect(!__PRINT_FLAG, 1)) 
		return 0;

	va_list ap;
	va_start(ap, fmt);
	int rst = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	return rst;
}
#endif

} // namespace gamed

#endif // GAMED_GS_GLOBAL_DBGPRT_H_

