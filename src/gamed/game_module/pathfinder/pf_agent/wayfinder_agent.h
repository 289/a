#ifndef __PF_WAY_FINDER_AGENT_H__
#define __PF_WAY_FINDER_AGENT_H__

#include <map>
#include "point.h"
#include "dgraph.h"

namespace pathfinder
{

class WayFinderAgent
{
protected:
private:
	float     _move_step;  // 移动速度
	int       _src_mapid;  // 起点所在地图ID
	int       _dest_mapid; // 终点所在地图ID
	PointI    _source;     // 起点
	PointI    _dest;       // 终点
	PF2DVECTOR _vsource;   // 起点坐标
	PF2DVECTOR _vdest;     // 终点坐标
	size_t    _cur_map;    // 当前地图
	size_t    _cur_pos;    // 当前位置
	Route     _map_route;  // 途径地图路由
	Route     _cur_route;  // 当前地图路由
	int       _state;      // AGENT状态


public:
	WayFinderAgent();
	~WayFinderAgent();

	void Init(int src_mapid, const PF2DVECTOR& source, int dest_mapid, const PF2DVECTOR& dest);
	void Release();

	void SetMoveStep(float move_step);
	bool Start();
	bool NextPath(std::vector<PF2DVECTOR>& path);
	bool GetCurStartPos(PF2DVECTOR& pos);
	bool GetCurEndPos(PF2DVECTOR& pos);
	int  GetState() const;

private:
  void CleanupRoute();
	bool UpdateCursor(); //更新位置信息
};

};

#endif // __PF_WAY_FINDER_AGENT_H__
