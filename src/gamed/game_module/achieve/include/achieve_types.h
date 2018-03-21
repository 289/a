#ifndef ACHIEVE_ACHIEVE_TYPE_H_
#define ACHIEVE_ACHIEVE_TYPE_H_

#include <stdio.h>
#include <stdint.h>
#include <set>
#include <vector>
#include <map>
#include <string>
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/base_packet.h"

namespace gamed
{
class AchieveInterface;
} // namespace gamed

namespace achieve
{

enum AchieveError
{
    // 0
    ERR_ACHIEVE_SUCC,
    ERR_ACHIEVE_ID,
    ERR_ACHIEVE_ACHIEVE,
    ERR_ACHIEVE_COND,
    ERR_ACHIEVE_ITEM,
};

enum AchieveFlag
{
    FLAG_UNFINISH_SHOW = 1,
};

enum AchieveOp
{
    ACHIEVE_OP_GREATER_EQUAL,
    ACHIEVE_OP_LESS_EQUAL,
};

enum StatusType
{
    STATUS_FINISH,
    STATUS_COMPLETE,
    STATUS_MODIFY,
};

class AchieveTempl;
typedef int32_t AchieveID;
typedef std::vector<AchieveID> AchieveIDVec;
typedef std::vector<const AchieveTempl*> AchieveVec;
typedef std::map<AchieveID, const AchieveTempl*> AchieveMap;

typedef gamed::AchieveInterface Player;

#define CHECK_INRANGE(value, min, max) \
	if (value < min || value > max) \
	{ \
		return false; \
	}
#define CHECK_VALIDITY(value) \
	if (!value.CheckDataValidity()) \
	{ \
		return false; \
	}
#define CHECK_VEC_VALIDITY(value) \
	for (size_t i = 0; i < value.size(); ++i) \
	{ \
		if (!value[i].CheckDataValidity()) \
		{ \
			return false; \
		} \
	}

extern bool __ACHIEVE_PRINT_FLAG;
inline void __SETPRTFLAG(bool flag)
{
	__ACHIEVE_PRINT_FLAG = flag;
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
#ifndef CLIENT_SIDE
	if (!__ACHIEVE_PRINT_FLAG)
		return 0;
#endif

	va_list ap;
	va_start(ap, fmt);
	int rst = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	return rst;
}
#endif

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_TYPE_H_
