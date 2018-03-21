#include "grid.h"

#include "shared/base/base_define.h"
#include "shared/base/assertx.h"

namespace gamed {

namespace 
{

	bool spec_overlap(const rect& large, const rect& small)
	{
		return large.IsIn(small.left,small.top) ||
			large.IsIn(small.left,small.bottom) ||
			large.IsIn(small.right,small.top) ||
			large.IsIn(small.right,small.bottom);
	}

}

Grid::Grid()
	: pslices_table_(NULL),
	  grid_region_(0.f, 0.f, 0.f, 0.f),
	  local_region_(0.f, 0.f, 0.f, 0.f),
	  slice_step_(0.f),
	  inv_step_(0.f)
{
}

Grid::~Grid()
{
	SAFE_DELETE_ARRAY(pslices_table_);	
}

void Grid::Release()
{
	SAFE_DELETE_ARRAY(pslices_table_);
	grid_region_  = rect(0.f, 0.f, 0.f, 0.f);
	local_region_ = rect(0.f, 0.f, 0.f, 0.f);
	slice_step_   = 0.f;
	inv_step_     = 0.f;
}

const Grid& Grid::operator=(const Grid& rhs)
{
	Assert((pslices_table_ == 0) && (&rhs != this));
	Assert(rhs.pslices_table_);

	grid_region_  = rhs.grid_region_;
	local_region_ = rhs.local_region_;
	inner_region_ = rhs.inner_region_;

	slice_step_   = rhs.slice_step_;
	inv_step_     = rhs.inv_step_;

	slice_count_  = rhs.slice_count_;
	reg_row_      = rhs.reg_row_;
	reg_column_   = rhs.reg_column_;

	int total      = reg_row_ * reg_column_;
	pslices_table_ = new Slice[total];
	memcpy(pslices_table_, rhs.pslices_table_, total * sizeof(Slice));

#ifndef NDEBUG
	for(size_t i = 0; i < (size_t)total; ++i)
	{
		Assert(pslices_table_[i].NumberOfPlayer() == 0);
		Assert(pslices_table_[i].NumberOfNPC() == 0);
	}
#endif 

	return *this;
}

bool Grid::Create(int row, int column, float step, float sx, float sy)
{
	Assert(pslices_table_ == NULL);
	grid_region_.left   = sx;
	grid_region_.top    = sy;
	grid_region_.right  = sx + column * step;
	grid_region_.bottom = sy + row * step;

	inv_step_     = 1.f/step;
	slice_step_   = step;
	reg_row_      = row;
	reg_column_   = column;

	int total      = row * column;
	pslices_table_ = new Slice[total];
	for (int i = 0; i < total; ++i)
	{
		float left = (i % column) * step + sx;
		float top  = (i / column) * step + sy;
		pslices_table_[i].slice_range_ = rect(left, top, left + step, top + step);
	}
	slice_count_ = total;

	return true;
}

bool Grid::SetRegion(const rect& local_rt, float border_size)
{
	if (local_rt.Width() <=0 || local_rt.Height() <=0) return false;
	if (!grid_region_.IsIn(local_rt)) return false;

	local_region_ = local_rt;

	// 重新计算每个格子所处的地位
	rect ne_region     = local_region_;   // 去除边界的区域
	ne_region.left    += slice_step_;
	ne_region.top     += slice_step_;
	ne_region.bottom  -= slice_step_;
	ne_region.right   -= slice_step_;

	rect in_region     = local_region_;   // 内部区域 （与其他服务器一般不搭界的区域）
	in_region.left    += border_size + slice_step_;
	in_region.top     += border_size + slice_step_;
	in_region.bottom  -= border_size + slice_step_;
	in_region.right   -= border_size + slice_step_;

	rect out_region    = local_region_;   // 外部敏感区（超出这个区域的就不再与本服务器相关）
	out_region.left   -= border_size;
	out_region.top    -= border_size;
	out_region.bottom += border_size;
	out_region.right  += border_size;

	int c1=0, c2=0, c3=0;
	for (int i = 0; i < slice_count_; ++i)
	{
		rect rt = pslices_table_[i].slice_range();
		Assert(spec_overlap(local_region_, rt) ? 1 : (local_region_.IsOverlap(rt) == 0 ? 1 : 0));
		if (!local_region_.IsOverlap(rt))
		{
			//不相干区域
			pslices_table_[i].flag_ |= Slice::OUTSIDE;
			if(out_region.IsOverlap(rt))
			{
				pslices_table_[i].flag_ |= Slice::SENSITIVE;
			}
			continue;
		}
		++c1;

		pslices_table_[i].flag_ |= Slice::INSIDE;
		if (in_region.IsIn(rt)) continue;       // 在内部
		++c2;

		pslices_table_[i].flag_ |= Slice::BORDER;
		if (ne_region.IsIn(rt)) continue;       // 在边界处
		++c3;

		pslices_table_[i].flag_ |= Slice::EDGE; // 就在边上
	}

	return true;
}

Slice* Grid::Locate(float x, float y) const
{
	ASSERT(grid_region_.IsIn(x, y));
	int ofx = (int)((x - grid_region_.left) * inv_step_);
	int ofy = (int)((y - grid_region_.top) * inv_step_);
	unsigned int offset  = (unsigned int)(ofx + ofy * reg_column_);
	if (offset >= (unsigned int)slice_count_) return NULL;

	return pslices_table_ + offset;
}

Slice* Grid::Locate(float x, float y, int &rx, int &ry) const
{
	Assert(grid_region_.IsIn(x, y));
	int ofx = (int)((x - grid_region_.left) * inv_step_);
	int ofy = (int)((y - grid_region_.top) * inv_step_);
	rx = ofx; ry = ofy;
	unsigned int offset  = (unsigned int)(ofx + ofy * reg_column_);
	if (offset >= (unsigned int)slice_count_) return NULL;

	return pslices_table_ + offset;
}

} // namespace gamed
