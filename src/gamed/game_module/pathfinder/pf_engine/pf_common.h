#ifndef __PF_COMMON_H__
#define __PF_COMMON_H__

#include <math.h>
#include <stdlib.h>
#include "point.h"
#include "types.h"

namespace pathfinder
{

#define REPULSION_DISTANCE 0.03f    // 小于该距离的2个点可以合并为一个点

enum
{       
	PF_ENGINE_STATE_IDLE,           // 空闲状态
	PF_ENGINE_STATE_ERROR,          // 出错状态
	PF_ENGINE_STATE_SETUP,          // 初始状态
	PF_ENGINE_STATE_SEARCHING,      // 正在搜索
	PF_ENGINE_STATE_FOUND,          // 找到目标
	PF_ENGINE_STATE_NOPATH,         // 不能到达
	PF_ENGINE_STATE_EXCEED,         // 超过最大遍历节点数
};

enum    
{       
	PF_AGENT_STATE_INIT,            // 初始状态
	PF_AGENT_STATE_UNKNOWN,         // 未知状态
	PF_AGENT_STATE_READY,           // 准备状态
	PF_AGENT_STATE_INVALID_MAP,     // 无效地图
	PF_AGENT_STATE_INVALID_POS,     // 无效位置
	PF_AGENT_STATE_CLOSE,           // 寻路结束
};

enum PF_DIR
{
	PF_NEIGHBOR_LEFT,
	PF_NEIGHBOR_RIGHT,
	PF_NEIGHBOR_TOP,
	PF_NEIGHBOR_BOTTOM,
	PF_NEIGHBOR_TOPRIGHT,
	PF_NEIGHBOR_BOTTOMRIGHT,
	PF_NEIGHBOR_TOPLEFT,
	PF_NEIGHBOR_BOTTOMLEFT,

	PF_NEIGHBOR_COUNT
};

const float PF_ROOT_TWO = 1.414f;
extern int PF2D_NeighborDist[];
extern float PF2D_NeighborCost[]; 
extern int PF2D_NeighborCost_I[];

inline PointI POINTF_2_POINTI(const PointF& pt)
{
	return PointI((int)(pt.x), (int)(pt.z));
}

inline PointI POINTF_2_POINTI_NEAR(const PointF& pt)
{
  return PointI((int)(pt.x + 0.5f), (int)(pt.z + 0.5f));
}

inline PointF POINTI_2_POINTF(const PointI& pt)
{
	return PointF((int)(pt.x) + 0.5f, (int)(pt.z) + 0.5f);
}


/**
 * @brief: 估算距离的三种算法
 * 1) Manhattan--曼哈顿算法;
 * 2) Euclidean--欧氏算法;
 * 3) Diagonal---斜边算法;
 */

inline float HeuristicDist(const PointI& pt1, const PointI& pt2)
{
	//return (float)(fabs((float)(pt1.x - pt2.x)) + fabs((float(pt1.z - pt2.z))));

	short diffX = abs(pt2.x - pt1.x);
	short diffZ = abs(pt2.z - pt1.z);
	short max_diff = diffX > diffZ ? diffX : diffZ;
	short min_diff = diffX < diffZ ? diffX : diffZ;
	short diffValue = max_diff - min_diff;
	return float(min_diff * PF_ROOT_TWO + float(diffValue));
}

inline float GetManhDist(const PointI& pt1, const PointI& pt2)
{
	return (float)(fabs((float)(pt1.x - pt2.x)) + fabs((float)(pt1.z - pt2.z)));
}

inline float GetEuclDist(const PointI& pt1, const PointI& pt2)
{
	return (sqrtf((pt1.x - (float)pt2.x)*(pt1.x - pt2.x) + (pt1.z - pt2.z)*(pt1.z - pt2.z)));
}

inline float GetDiagDist(const PointI& pt1, const PointI& pt2)
{
	short diffX = abs(pt2.x - pt1.x);
	short diffZ = abs(pt2.z - pt1.z);
	short max_diff = diffX > diffZ ? diffX : diffZ;
	short min_diff = diffX < diffZ ? diffX : diffZ;
	short diffValue = max_diff - min_diff;
	return float(min_diff * PF_ROOT_TWO + float(diffValue));
}

/**
 * @func  HeuristicDistance
 * @brief 抽象图上两点间启发式距离的估值函数
 * @brief 如果两点位于同一地图，则使用斜边距离算法计算
 * @brief 如果两点不在同一地图，则统一使用距离1代替
 */
template <class T>
inline float GetHeuristic(const T* source, const T* target)
{
	if (source->GetOwner() != target->GetOwner())
		return 1.0f;

	PointI ptStart = source->GetCoord();
	PointI ptGoal  = target->GetCoord();
	short diffX = fabsf(ptGoal.x - ptStart.x);
	short diffZ = fabsf(ptGoal.z - ptStart.z);
	short minDiff = diffX < diffZ ? diffX : diffZ;
	short maxDiff = diffX < diffZ ? diffZ : diffX;
	return (float)((maxDiff - minDiff) + (minDiff * PF_ROOT_TWO));
}

/**
 * @func  RotateVector
 * @brief 顺时针旋转矢量
 * @param src_vt  源矢量
 * @param degree  旋转角度
 * @ret   旋转得到的新矢量
 */
inline PF2DVECTOR ClockWiseRotateVector(const PF2DVECTOR& src_vt, float degree)
{
	//顺时针矢量旋转公式
	//X =  x * cos(A) + y * sin(A);
	//Y = -x * sin(A) + y * cos(A);

	PF2DVECTOR __src = src_vt;
	PF2DVECTOR __dest;
	__dest.x = __src.x * cos(degree) + __src.z * sin(degree);
	__dest.z = -1.0f * __src.x * sin(degree) + __src.z * cos(degree);
	return __dest;
}

/**
 * @func  RotateVector
 * @brief 逆时针旋转矢量
 * @param src_vt  源矢量
 * @param degree  旋转角度
 * @ret   旋转得到的新矢量
 */
inline PF2DVECTOR AntiClockWiseRotateVector(const PF2DVECTOR& src_vt, float degree)
{
	//逆时针矢量旋转公式
	//X = x * cos(A) - y * sin(A)
	//Z = x * sin(A) + y * cos(A)

	PF2DVECTOR __src = src_vt;
	PF2DVECTOR __dest;
	__dest.x = __src.x * cos(degree) - __src.z * sin(degree);
	__dest.z = __src.x * sin(degree) + __src.z * cos(degree);
	return __dest;
}

};

#endif // __PF_COMMON_H__
