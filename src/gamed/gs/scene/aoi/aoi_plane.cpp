#include "aoi_plane.h"

#include "shared/logsys/logging.h"


namespace gamed {

namespace  
{
} // anonymous 

AoiPlane::AoiPlane()
	: far_vision_(0),
	  w_vision_(0.f),
	  near_vision_(0),
	  true_vision_(0)
{
}

AoiPlane::~AoiPlane()
{
	off_list_.clear();
	far_vision_  = 0;
	w_vision_    = 0.f;
	near_vision_ = 0;
	true_vision_ = 0;
}

bool AoiPlane::Init(const InitData& data)
{
	if (!grid_.Create(data.row, data.column, data.step, data.startX, data.startY))
	{
		LOG_ERROR << "aoi init error: grid Create() ERROR!";
		return false;
	}

	if (!BuildSliceMask(data.near_vision, data.far_vision))
	{
		LOG_ERROR << "BuildSliceMask failed!";
		return false;
	}

	return true;
}

AoiSlice* AoiPlane::Locate(float x, float y) const
{
	return grid_.Locate(x, y);
}

bool AoiPlane::BuildSliceMask(float near, float far)
{
	if (far < near) return false;

	off_list_.clear();
	float inv_step = grid_.inv_step();
	int n1   = (int)(near * inv_step);
	int f1   = (int)(far * inv_step);
	int tf1  = (int)((far - grid_.slice_step()) * inv_step);

	if (fabs(near - n1 * grid_.slice_step()) > 1e-3) ++n1;
	if (fabs(far - f1 * grid_.slice_step()) > 1e-3) ++f1;

	for (int i = 1; i <= f1; ++i)
	{
		for(int j = -i; j < i; ++j)
		{
			insert_unique(off_list_, off_node_t(grid_, j,-i));
			insert_unique(off_list_, off_node_t(grid_, i, j));
			insert_unique(off_list_, off_node_t(grid_,-j, i));
			insert_unique(off_list_, off_node_t(grid_,-i,-j));
		}
		if(n1 == i) near_vision_  = off_list_.size();
		if(tf1 == i) true_vision_ = off_list_.size();
	}
	far_vision_ = off_list_.size();
	w_vision_   = far;

	return true;
}

} // namespace gamed
