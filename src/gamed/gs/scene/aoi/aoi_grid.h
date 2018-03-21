#ifndef GAMED_GS_SCENE_AOI_GRID_H_
#define GAMED_GS_SCENE_AOI_GRID_H_

#include "shared/base/noncopyable.h"
#include "gs/global/math_types.h"

#include "aoi_slice.h"


namespace gamed {

class AoiSlice;

/**
 * @brief AoiGrid
 */
class AoiGrid : shared::noncopyable
{
public:
	AoiGrid();
	~AoiGrid();

	bool Create(int row, int column, float step, float startX, float startY);
	void Release();

	AoiSlice* Locate(float x, float y) const;

	inline void  GetSlicePos(AoiSlice *pPiece, int& x, int& y) const;

	// Member variables op functions
	inline int   reg_column() const;
	inline int   reg_row() const;
	inline float inv_step() const;
	inline float slice_step() const;


private:
	AoiSlice* pslices_table_; // 所有分区组成的表
	rect      grid_region_;   // 整个分区所管辖的区域
	float     slice_step_;    // 步长的大小
	float     inv_step_;      // 步长的大小的倒数
	int       slice_count_;
	int       reg_row_;
	int       reg_column_;
};

///
/// inline func
///
inline void AoiGrid::GetSlicePos(AoiSlice *pPiece, int& x, int& y) const
{
	x =(int)((pPiece->slice_range().left - grid_region_.left + 0.1f) * inv_step_);
	y =(int)((pPiece->slice_range().top - grid_region_.top + 0.1f) * inv_step_);
}

inline int AoiGrid::reg_column() const
{
	return reg_column_;
}

inline int AoiGrid::reg_row() const
{
	return reg_row_;
}

inline float AoiGrid::inv_step() const
{
	return inv_step_;
}

inline float AoiGrid::slice_step() const
{
	return slice_step_;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_AOI_GRID_H_
