#ifndef GAMED_GS_SCENE_AOI_PLANE_H_
#define GAMED_GS_SCENE_AOI_PLANE_H_

#include <vector>
#include <algorithm>

#include "shared/base/noncopyable.h"

#include "aoi_grid.h"


namespace gamed {

/**
 * @brief AOI网格的操作类
 */
class AoiPlane : shared::noncopyable
{
public:
	struct InitData
	{
		int   row;
		int   column;
		float step;
		float startX;
		float startY;
		float near_vision;
		float far_vision;
	};

	AoiPlane();
	~AoiPlane();

	bool Init(const InitData& initdata);
	AoiSlice* Locate(float x, float y) const;

	template <typename FUNC>
	inline void ForEachSlice(AoiSlice* pStart, FUNC func, int vlevel = 0);

	template <typename ENTER, typename LEAVE>
	inline void MoveBetweenSlice(AoiSlice* pPiece, AoiSlice* pNewPiece, ENTER enter, LEAVE leave);


private:
	struct off_node_t
	{
		off_node_t(AoiGrid& grid, int offset_x, int offset_y)
			: x_off(offset_x),
			  y_off(offset_y)
		{
			idx_off = (offset_y * grid.reg_column()) + offset_x;
		}

		bool operator==(const off_node_t& rhs) const
		{
			return rhs.idx_off == idx_off;
		}

		int idx_off;
		int x_off;
		int y_off;
	};
	typedef std::vector<off_node_t> OffListVec;


private:
	bool BuildSliceMask(float near, float far);
	inline void insert_unique(OffListVec& list, const off_node_t& node);
	inline bool check_index(const AoiGrid* g, int x, int y, const off_node_t& node);


private:
	AoiGrid    grid_;

	OffListVec off_list_;

	int        far_vision_;  // 最远的距离所涵盖的范围（格子范围）
	float      w_vision_;    // 视野距离，和初始化时传入的far_vision对应

	int        near_vision_; // 最近的距离所涵盖的范围（格子范围）
	int        true_vision_; // 完全可见的范围，暂时没有使用
};

///
/// inline func
///
template <typename FUNC>
inline void AoiPlane::ForEachSlice(AoiSlice* pStart, FUNC func, int vlevel)
{
	int total = vlevel ? near_vision_ : far_vision_;
	int slice_x, slice_y;
	grid_.GetSlicePos(pStart, slice_x, slice_y);
	for (int i = 0; i < total; ++i)
	{
		off_node_t& node = off_list_[i];
		int nx = slice_x + node.x_off;
		int ny = slice_y + node.y_off;
		if (nx < 0 || ny < 0 || nx >= grid_.reg_column() || ny >= grid_.reg_row())
			continue;
		AoiSlice* pNewPiece = pStart + node.idx_off;
		func(i, pNewPiece);
	}
}

/**
 * @brief MoveBetweenSlice 
 * （1）一个对象在两个格子间移动，判断该对象离开了哪些格子的视野，会调用相应的enter和leave函数对象
 */
template <typename ENTER, typename LEAVE>
inline void AoiPlane::MoveBetweenSlice(AoiSlice* pPiece, AoiSlice* pNewPiece, ENTER enter, LEAVE leave)
{
	if (pPiece == pNewPiece)
		return;

	int i, ox, oy, nx, ny;
	AoiGrid* pGrid = &grid_;
	pGrid->GetSlicePos(pPiece, ox, oy);
	pGrid->GetSlicePos(pNewPiece, nx, ny);
	float vision = w_vision_ + pGrid->slice_step() - 1e-3;
	float dis    = pNewPiece->Distance(pPiece);
	if (dis > vision)
	{
		// 本格的无法看见，所以要进行离开本格的操作，后面的循环并没有判断本格
		leave(pPiece);
		enter(pNewPiece);
		if (dis > vision*2)
		{
			for (i = 0; i < far_vision_; ++i)
			{
				const off_node_t& node = off_list_[i];
				AoiSlice* pTmpPiece = pPiece + node.idx_off;
				leave(pTmpPiece);
			}

			for (i = 0; i < far_vision_; ++i)
			{
				const off_node_t& node = off_list_[i];
				AoiSlice* pTmpPiece = pNewPiece + node.idx_off;
				enter(pTmpPiece);
			}
			return;
		}
	}

	for (i = 0; i < far_vision_; ++i)
	{
		const off_node_t& node = off_list_[i];
		if (check_index(pGrid, ox, oy, node))
		{
			AoiSlice* pTmpPiece = pPiece + node.idx_off;
			if (pTmpPiece->Distance(pNewPiece) > vision)
			{
				leave(pTmpPiece);
			}
		}

		if (check_index(pGrid, nx, ny, node))
		{
			AoiSlice* pTmpPiece = pNewPiece + node.idx_off;
			if (pTmpPiece->Distance(pPiece) > vision)
			{
				enter(pTmpPiece);
			}
		}
	}
}

inline void AoiPlane::insert_unique(OffListVec& list, const off_node_t& node)
{
	if (std::find(list.begin(), list.end(), node) == list.end())
	{
		list.push_back(node);
	}
}

inline bool AoiPlane::check_index(const AoiGrid* g, int x, int y, const off_node_t& node)
{
	int nx = x + node.x_off;
	if (nx < 0 || nx >= g->reg_column()) 
		return false;

	int ny = y + node.y_off;
	if (ny < 0 || ny >= g->reg_row()) 
		return false;

	return true;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_AOI_PLANE_H_
