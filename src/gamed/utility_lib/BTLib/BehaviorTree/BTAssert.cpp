#include "BTAssert.h"

#include <stdio.h>
#include <assert.h>


namespace BTLib {

void __show__(const char* szTemp)
{
	fprintf(stderr, "Assert:%s",szTemp);
	//是否需要记log,抛异常?
	assert(false);
}   

void __assert__(const char* file, uint32_t line, const char* func, const char* expr)
{
	char szTemp[1024] = {0};
	sprintf(szTemp, "[%s]-[%d]-[%s]-[%s]\n", file, line, func, expr);
	__show__(szTemp) ;
}

} // namespace BTLib
