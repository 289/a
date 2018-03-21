#ifndef __PF_TYPES_H__
#define __PF_TYPES_H__

#include <stdint.h>
#include "point.h"

namespace pathfinder
{
	class  GNode;
	struct VexNode;

	typedef Point<short> PointI;
	typedef Point<float> PointF;
	typedef std::vector<PointI> Path;
	typedef std::vector<PointF> PATH;
	typedef std::vector<const GNode*> ROUTE;
	typedef std::vector<const VexNode*> Route;
};

#endif // __PF_TYPES_H__
