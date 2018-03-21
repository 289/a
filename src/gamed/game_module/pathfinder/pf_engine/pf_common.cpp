#include "pf_common.h"

namespace pathfinder
{

int PF2D_NeighborDist[PF_NEIGHBOR_COUNT * 2] =
{
	-1,  0,   //left
	+1,  0,   //right
	0,   +1,  //top
	0,   -1,  //bottom
	+1,  +1,  //topright
	+1,  -1,  //bottomright
	-1,  +1,  //topleft
	-1,  -1   //bottomleft
};

float PF2D_NeighborCost[PF_NEIGHBOR_COUNT] = 
{
	1.0f,
	1.0f,
	1.0f,
	1.0f,
	(float)PF_ROOT_TWO,
	(float)PF_ROOT_TWO,
	(float)PF_ROOT_TWO,
	(float)PF_ROOT_TWO
};

int PF2D_NeighborCost_I[PF_NEIGHBOR_COUNT] = 
{
  5,  // 1.0f * 10 / 2
  5,
  5,
  5,
  7,  // 1.4f * 10 / 2
  7,
  7,
  7
};

};
