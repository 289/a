#ifndef GAMED_UTILITY_LIB_BTLIB_BTASSERT_H_
#define GAMED_UTILITY_LIB_BTLIB_BTASSERT_H_

#include <stdint.h>

namespace BTLib {

// New define for Assert
void __assert__ (const char* file, uint32_t line, const char* func, const char* expr) ;

} // namespace BTLib

#define BT_ASSERT(expr) {if(!(expr)){BTLib::__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr);}}

#endif // GAMED_UTILITY_LIB_BTLIB_BTASSERT_H_
