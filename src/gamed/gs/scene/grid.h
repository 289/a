#ifndef GAMED_GS_SCENE_GRID_H_
#define GAMED_GS_SCENE_GRID_H_

#include "gs/global/math_types.h"
#include "gs/scene/slice.h"

namespace gamed {

class Grid
{
public:
	Grid();
	~Grid();

	bool    Create(int row, int column, float step, float startX, float startZ);
	bool    SetRegion(const rect& local_rt, float border_size);
	void    Release();

	Slice*  Locate(float x, float y) const;
	Slice*  Locate(float x, float y, int &rx, int &rz) const;

	inline Slice*  GetSlice(int index) const;
	inline Slice*  GetSlice(int x, int y) const;
	inline int     GetSliceIndex(Slice *pPiece) const;
	inline void    GetSlicePos(Slice *pPiece, int &x,int &y) const;

	inline void    Index2Pos(int index, int &x,int &y) const;
	inline bool    IsOutsideGrid(float x, float y) const;
	inline bool    IsLocal(float x, float y) const;

	// Member variables op functions
	inline void    set_inner_region(const rect& region);
	inline int     reg_column() const;
	inline int     reg_row() const;
	inline float   inv_step() const;
	inline float   slice_step() const;

	const Grid& operator=(const Grid& rhs);


private:
	Slice*    pslices_table_; // 所有分区组成的表
	rect      grid_region_;   // 整个分区所管辖的区域
	rect      local_region_;  // 本地区域，超过这个区域应该进行服务器转移
	rect      inner_region_;  // 内部区域，这个区域只由自己负责
	float     slice_step_;    // 步长的大小
	float     inv_step_;      // 步长的大小的倒数
	int       slice_count_;
	int       reg_row_;
	int       reg_column_;
};

///
/// inline func
///
inline Slice* Grid::GetSlice(int index) const
{
	return pslices_table_ + index;
}

inline Slice* Grid::GetSlice(int x, int y) const
{
	return pslices_table_ + x + y * reg_column_;
}

inline int Grid::GetSliceIndex(Slice *pPiece) const 
{
	return pPiece - pslices_table_;
}

inline void Grid::GetSlicePos(Slice *pPiece, int &x,int &y) const
{
	x =(int)( (pPiece->slice_range().left - grid_region_.left + 0.1f) * inv_step_ );
	y =(int)( (pPiece->slice_range().top - grid_region_.top + 0.1f) * inv_step_ );
}

inline void Grid::Index2Pos(int index, int &x, int &y) const 
{ 
	x = index % reg_column_; 
	y = index / reg_column_;
}

inline bool Grid::IsOutsideGrid(float x, float y) const 
{ 
	return grid_region_.IsOut(x, y);
}

inline bool Grid::IsLocal(float x, float y) const 
{ 
	return local_region_.IsIn(x, y);
}

inline void Grid::set_inner_region(const rect& region)
{
	inner_region_ = region;
}

inline int Grid::reg_column() const
{
	return reg_column_;
}

inline int Grid::reg_row() const
{
	return reg_row_;
}

inline float Grid::inv_step() const
{
	return inv_step_;
}

inline float Grid::slice_step() const
{
	return slice_step_;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_GRID_H_
