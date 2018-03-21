#ifndef SHARED_BASE_ASSERTX_H_
#define SHARED_BASE_ASSERTX_H_

#include <assert.h>


// reference BOOST_STATIC_ASSERT
namespace shared {
	template<bool x> struct STATIC_ASSERTION_FAILURE;
	template<> struct STATIC_ASSERTION_FAILURE<true>{};
	template<int x> struct static_assert_test{};
}
#define SHARED_STATIC_ASSERT(B) \
	typedef shared::static_assert_test< \
		sizeof(shared::STATIC_ASSERTION_FAILURE<(bool)(B)>) \
		> static_assert_typedef_ ## __LINE__


// New define for Assert
void __assert__ (const char* file, int line, const char* func, const char* expr) ;
void __assertex__ (const char* file, int line, const char* func, const char* expr, const char* msg) ;

#ifdef NDEBUG
 #define Assert(expr) {if(!(expr)){__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr);}}
 #define AssertEx(expr,msg) {if(!(expr)){__assertex__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg);}}
#else // !NDEBUG
 #define Assert(expr) {assert(expr);}
 #define AssertEx(expr,msg) {assert(expr);}
#endif // NDEBUG

#define ASSERT(expr) Assert(expr)
#define ASSERT_EX(expr,msg) AssertEx(expr,msg)

#endif //SHARED_BASE_ASSERTX_H_
