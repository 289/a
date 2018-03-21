#include "aoi_grid.h"

#include "shared/base/base_define.h"


namespace gamed {

AoiGrid::AoiGrid()
	: pslices_table_(NULL),
	  grid_region_(0.f, 0.f, 0.f, 0.f),
	  slice_step_(0.f),
	  inv_step_(0.f),
	  slice_count_(0),
	  reg_row_(0),
	  reg_column_(0)
{
}

AoiGrid::~AoiGrid()
{
	Release();
}

void AoiGrid::Release()
{
	SAFE_DELETE_ARRAY(pslices_table_);
	grid_region_ = rect(0.f, 0.f, 0.f, 0.f);
	slice_step_  = 0.f;
	inv_step_    = 0.f;
	slice_count_ = 0.f;
	reg_row_     = 0;
	reg_column_  = 0;
}

bool AoiGrid::Create(int row, int column, float step, float sx, float sy)
{
	ASSERT(pslices_table_ == NULL);
	grid_region_.left   = sx;
	grid_region_.top    = sy;
	grid_region_.right  = sx + column * step;
	grid_region_.bottom = sy + row * step;

	inv_step_   = 1.f/step;
	slice_step_ = step;
	reg_row_    = row;
	reg_column_ = column;

	int total      = row * column;
	pslices_table_ = new AoiSlice[total];
	for (int i = 0; i < total; ++i)
	{
		float left = (i % column) * step + sx;
		float top  = (i / column) * step + sy;
		rect tmp_rt(left, top, left + step, top + step);
		pslices_table_[i].set_slice_range(tmp_rt);
	}
	slice_count_ = total;

	return true;
}

AoiSlice* AoiGrid::Locate(float x, float y) const
{
	ASSERT(grid_region_.IsIn(x, y));
	int ofx = (int)((x - grid_region_.left) * inv_step_);
	int ofy = (int)((y - grid_region_.top) * inv_step_);
	unsigned int offset  = (unsigned int)(ofx + ofy * reg_column_);
	if (offset >= (unsigned int)slice_count_) return NULL;

	return pslices_table_ + offset;
}

} // namespace gamed
