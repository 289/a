#ifndef __PF_CHASE_AGENT_H__
#define __PF_CHASE_AGENT_H__

#include "point.h"
#include "path_track.h"

namespace pathfinder
{

class MoveMap;
class ChaseAgent
{
private:
	MoveMap*   _movemap;      // 通路图
	float      _range_square; // 怪物攻击距离的平方
	PF2DVECTOR _vcur_pos;     // 怪物当前位置(大世界坐标)
	PointF     _cur_pos;      // 怪物当前位置
	PointF     _goal_pos;     // 目标位置
	PF2DVECTOR _vgoal_pos;    // 目标位置(大世界坐标)
	PathTrack  _path;         // 怪物追寻路径
	int        _agent_state;  // AGENT状态

public:
	ChaseAgent();
	~ChaseAgent();

	void Init(int mapid, const PF2DVECTOR& vcurpos);
	void SetGoal(const PF2DVECTOR& vgoal, float distance);

	bool StartChase();
	bool StepMove(float step);

	bool TouchGoal() const;
	PF2DVECTOR GetCurPos() const;
	PF2DVECTOR GetTarget() const;

	void SetState(int state);
	int GetState() const;
};

};

#endif // __PF_CHASE_AGENT_H__
