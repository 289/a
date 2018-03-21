#ifndef GAMED_CLIENT_PROTO_TYPES_H_
#define GAMED_CLIENT_PROTO_TYPES_H_

#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/packet_util.h"
#include "gs/global/math_types.h"


namespace gamed {

namespace movement
{
	enum MoveMode
	{
		MOVE_MODE_WALK      = 0x00, 
		MOVE_MODE_RUN       = 0x01, 
		MOVE_MODE_RETURN    = 0x02,
	};
} // namespace movement

using namespace shared::net;

class A2DVECTOR_PACK : public A2DVECTOR
{
public:
	A2DVECTOR_PACK()
		: A2DVECTOR(0.f, 0.f)
	{ }

	A2DVECTOR_PACK(const A2DVECTOR& rhs)
		: A2DVECTOR(rhs.x, rhs.y)
	{ }

	const A2DVECTOR_PACK& operator=(const A2DVECTOR& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	const A2DVECTOR_PACK& operator=(const A2DVECTOR_PACK& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	NESTED_DEFINE(x, y);
};

} // namespace gamed

#endif // GAMED_CLIENT_PROTO_TYPES_H_
