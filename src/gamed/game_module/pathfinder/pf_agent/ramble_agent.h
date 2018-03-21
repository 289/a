#ifndef __PF__RAMBLE_AGENT_H__
#define __PF__RAMBLE_AGENT_H__

#include "point.h"
#include "path_track.h"

namespace pathfinder
{

/**
 * @brief 怪物巡逻,怪物在指定范围内随机移动,限直线移动
 */

class MoveMap;
class RambleAgent
{
private:
	MoveMap*  _movemap;     // 通路图
	float     _move_step;   // 步伐长度
	float     _move_range;  // 巡逻范围
	PointF    _cur_pos;     // 怪物当前位置
	PointF    _center_pos;  // 巡逻中心点
	PointF    _goal_pos;    // 怪物目标位置
	PathTrack _ramble_path; // 怪物巡逻路径

	short     _dir_last;    // 怪物无路可走时选择的随机方向
	int       _agent_state; // AGENT状态

public:
	RambleAgent();
	~RambleAgent();

public:
	void Init(int mapid, const PF2DVECTOR& vcurpos);
	void SetRambleInfo(const PF2DVECTOR& vcenter, float range);
	void SetMoveStep(float move_step);

	bool StartRamble();
	bool StepMove();
	bool GetToGoal() const;

	PF2DVECTOR GetCurPos() const;
	PF2DVECTOR GetGoalPos() const;

	void SetState(int state);
	int GetState() const;

private:
	void BuildPath();
};

};

#endif // __PF__RAMBLE_AGENT_H__
