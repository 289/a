#ifndef __PF_PATH_FINDER_H__
#define __PF_PATH_FINDER_H__

#include <vector>
#include "point.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace net
}; // namespace shared

namespace pathfinder
{

/**
 * @func  InitGlobalTransTable
 * @brief 初始化地图间传送关系表
 * @brief 服务器使用
 * @param file_path 传送表数据
 * @ret   初始化是否成功
 */
bool InitGlobalTransTable(const char* file_path);

/**
 * @func  LoadMoveMap
 * @brief 服务器使用
 * @param mapid     地图ID
 * @ret   加载是否成功
 */
bool LoadMoveMap(int mapid, const char* filepath=NULL);

/**
 * @func  InitGlobalTransTable
 * @brief 初始化地图间传送关系表
 * @brief 客户端使用
 * @param buffer 传送表数据
 * @ret   初始化是否成功
 */
bool InitGlobalTransTable(shared::net::Buffer& buffer);

/**
 * @func  LoadMoveMap
 * @brief 客户端使用
 * @param mapid  地图ID
 * @param buffer 地图数据
 * @ret   加载是否成功
 */
bool LoadMoveMap(int mapid, shared::net::Buffer& buffer);

/**
 * @func  FreeMoveMap
 * @brief 释放通路图数据
 * @brief 客户端使用
 * @param mapid 地图ID
 * @ret   释放是否成功
 */
bool FreeMoveMap(int mapid);

/**
 * @func  Release
 * @brief 释放寻路模块所有资源
 * @brief 客户端和服务器共用
 */
void Release();

/**
 * @func  SearchPath
 * @brief 本地图本区域寻路
 * @brief 服务器和客户端共用
 * @param mapid  地图ID
 * @param src    起点坐标
 * @param dest   终点坐标
 * @param step   行走步长
 * @param path   寻路结果
 * @ret   找到路径返回true,否则返回false
 */
bool SearchPath(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& dest, float step, std::vector<PF2DVECTOR>& path);

/**
 * @func  IsWalkable
 * @brief 判断该位置是否可通过
 * @param mapid 地图ID
 * @param pos   位置
 * @ret   可通过返回ture,否则返回false
 */
bool IsWalkable(int mapid, const PF2DVECTOR& pos);

/**
 * @func  SetWalkable
 * @brief 修改地图上特定点的可达情况,实现动态通路图
 * @param pos 特定点坐标
 * @param walkable 可达情况
 * @ret   修改成功返回true,否则返回false
 */
bool SetWalkable(int mapid, const PF2DVECTOR& pos, bool walkable);

enum WalkableUnit
{
  WalkableUnit_1 = 1,             // 一个点
  //WalkableUnit_LeftTop_3x3,   // 以输入点为左上角,设置3x3点
  //WalkableUnit_LeftTop_5x5,   // 不说了，应该能看明白,5x5
  WalkableUnit_Center_3x3,    // 以输入点位中心,设置3x3点
  WalkableUnit_Center_5x5,    // 应该也能看明白,5x5
};
/**
 * @func  SetWalkable
 * @brief 修改地图上特定点的可达情况,实现动态通路图
 * @param pos 特定点坐标
 * @param unit 单元扩展方式
 * @param walkable 可达情况
 * @ret   修改成功返回true,否则返回false
 */
bool SetWalkable(int mapid, const PF2DVECTOR& pos, WalkableUnit unit, bool walkable);

/**
 * @func  LineTo
 * @brief 判断两点是否直线可达
 * @param mapid  地图ID
 * @param source 起点
 * @param dest   终点
 * @ret   可达返回true,否则返回false
 */
bool LineTo(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& dest);

/**
 * @func  GetGridSize
 * @brief 返回本地图最小单元格的大小
 * @param mapid 地图ID
 * @ret   最小单元格尺寸
 */
float GetGridSize(int mapid);

/**
 * @func  FindNearbyWalkablePos
 * @brief 找目标点附近可通过的点
 * @param pos    源位置(可达情况未知)
 * @param newpos 源位置附近可通过的点
 * @ret   找到返回true,否则返回false
 */
bool FindNearbyWalkablePos(int mapid, const PF2DVECTOR& pos, PF2DVECTOR& newpos);

/**
 * @func  IsInSameArea
 * @brief 判断两个点是否属于同一区域
 * @param mapid 地图ID
 */
bool IsInSameArea(int mapid, const PF2DVECTOR& pos1, const PF2DVECTOR& pos2);


class RambleAgent;
class ChaseAgent;
class WayFinderAgent;

/**
 * @class NpcRamble
 * @brief NPC巡逻
 */
class NpcRamble
{
private:
	RambleAgent* _agent;

public:
	NpcRamble();
	~NpcRamble();

	bool Start(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& center, float speed, float range);
	bool MoveOneStep();
	PF2DVECTOR GetCurPos() const;
	PF2DVECTOR GetGoalPos() const;
	bool GetToGoal() const;
};

/**
 * @class NpcChase
 * @brief NPC跟随/追逐
 */
class NpcChase
{
private:
	ChaseAgent* _agent;

public:
	NpcChase();
	~NpcChase();

	bool Start(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& target, float range);
	bool MoveOneStep(float step);
	PF2DVECTOR GetTarget() const;
	PF2DVECTOR GetCurPos() const;
	bool GetToGoal() const;
};

/**
 * @class NpcFixedRamble
 * @brief NPC定点巡逻
 */
class NpcFixedRamble
{
private:
	int _mapid;
	float _speed;
	float _range;
	PF2DVECTOR _center;
	RambleAgent* _agent;
public:
	NpcFixedRamble();
	~NpcFixedRamble();

	bool Init(int mapid, float speed, float range, const PF2DVECTOR& center);
	bool Start(const PF2DVECTOR& source);
	PF2DVECTOR GetGoalPos() const;
	void GetToGoal();
};

/**
 * @class WayFinder
 * @brief 客户端玩家寻路
 * @brief 跨地图寻路调用
 */
class WayFinder
{
private:
	WayFinderAgent* _agent;

public:
	WayFinder();
	~WayFinder();

	bool Start(int src_mapid, const PF2DVECTOR& src, int dest_mapid, const PF2DVECTOR& dest, float step);
	bool NextPath(std::vector<PF2DVECTOR>& path);

	bool GetCurStartPos(PF2DVECTOR& pos);//test
	bool GetCurEndPos(PF2DVECTOR& pos);//test
};

};

#endif // __PF_PATH_FINDER_H__
