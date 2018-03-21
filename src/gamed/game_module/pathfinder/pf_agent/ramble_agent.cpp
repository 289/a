#include <assert.h>
#include "shared/security/randomgen.h"
#include "ramble_agent.h"
#include "move_map.h"
#include "pf_common.h"
#include "pf_centre.h"

namespace pathfinder
{

using namespace shared::net; 

RambleAgent::RambleAgent() :
	_movemap(NULL),
	_move_step(0.0f),
	_move_range(0.0f),
	_dir_last(0),
	_agent_state(PF_AGENT_STATE_INIT)
{
}

RambleAgent::~RambleAgent()
{
}

void RambleAgent::Init(int mapid, const PF2DVECTOR& vcurpos)
{
	_movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!_movemap)
	{
		SetState(PF_AGENT_STATE_UNKNOWN);
		return;
	}

	/*
	PointF pt = _movemap->TransWorld2Map(vcurpos);
	if (!_movemap->IsPassable(pt))
	{
		SetState(PF_AGENT_STATE_INVALID_POS);
		return;
	}
	_cur_pos = pt;
	*/

	_cur_pos = _movemap->TransWorld2Map(vcurpos);
	SetState(PF_AGENT_STATE_READY);
}

void RambleAgent::SetRambleInfo(const PF2DVECTOR& vcenter, float range)
{
	PointF pt = _movemap->TransWorld2Map(vcenter);
	if (!_movemap->IsPassable(pt))
	{
		SetState(PF_AGENT_STATE_INVALID_POS);
		return;
	}

	_center_pos = pt;
	_move_range = range;
}

void RambleAgent::SetMoveStep(float move_step)
{
	_move_step = move_step;
}

bool RambleAgent::StartRamble()
{
	if (_agent_state != PF_AGENT_STATE_READY)
	{
		return false;
	}

	if (_move_range <= 0.0f || _move_step  <= 0.0f)
	{
		assert(false);
		return false;
	}

	float grid_size = _movemap->GetGridSize();
	float grid_count = _move_range / grid_size;

	PointI pt_min((short)(_center_pos.x - grid_count), (short)(_center_pos.z - grid_count));
	PointI pt_max((short)(_center_pos.x + grid_count), (short)(_center_pos.z + grid_count));

	short RAND_RANGE_X = pt_max.x - pt_min.x + 1;
	short RAND_RANGE_Z = pt_max.z - pt_min.z + 1;
	int32_t RAND_RANGE = RAND_RANGE_X * RAND_RANGE_Z;
	int32_t RAND_TIMES = 50;

	for (int i = 0; i < RAND_TIMES; ++ i)
	{
		PointI rand_pos;
		int rand_num = RandomGen::RandUniform(0, RAND_RANGE);
		rand_pos.x = pt_min.x + rand_num % RAND_RANGE_X;
		rand_pos.z = pt_min.z + rand_num / RAND_RANGE_X;

		if (rand_pos != POINTF_2_POINTI(_cur_pos) &&
			_movemap->IsPassable(rand_pos) &&
			_movemap->LineTo(POINTF_2_POINTI(_cur_pos), rand_pos))
		{
			_goal_pos = POINTI_2_POINTF(rand_pos);
			BuildPath();
			return true;
		}
	}

	//NPC无路径可走，随便选择一个方向往前走
	for (int i = 0; i < PF_NEIGHBOR_COUNT; ++ i)
	{
		short dir = (_dir_last + i) % PF_NEIGHBOR_COUNT;
		PointF next_pos = _cur_pos;
		next_pos.x += PF2D_NeighborDist[2*dir];
		next_pos.z += PF2D_NeighborDist[2*dir + 1];
		if (next_pos.x >= pt_min.x &&
			next_pos.z >= pt_min.z &&
			next_pos.x <= pt_max.x &&
			next_pos.z >= pt_max.z &&
			_movemap->IsPassable(next_pos))
		{
			_goal_pos = next_pos;
			BuildPath();
			return true;
		}
	}

	return false;
}

bool RambleAgent::StepMove()
{
	if (GetToGoal() || _ramble_path.GetLength() <= 1)
	{
		return false;
	}

	PF2DVECTOR next_pos;
	if (_ramble_path.GetNextPos(next_pos, _move_step) < 0.0f)
	{
		return false;
	}

	_cur_pos = _movemap->TransWorld2Map(next_pos);
    if (!_movemap->IsPassable(_cur_pos)) 
    {
        return false;
    }

	return true;
}

bool RambleAgent::GetToGoal() const
{
	/**
	 * 当前点和目标点坐标均小于REPULSION_DISTANCE时视为到达目标
	 */
	PointF offset = _goal_pos - _cur_pos;
    float grid_size = _movemap->GetGridSize();  
	return fabs(offset.x * grid_size) <= REPULSION_DISTANCE && fabs(offset.z * grid_size) <= REPULSION_DISTANCE;
}

PF2DVECTOR RambleAgent::GetCurPos() const
{
	return _movemap->TransMap2World(_cur_pos);
}

PF2DVECTOR RambleAgent::GetGoalPos() const
{
	return _movemap->TransMap2World(_goal_pos);
}

void RambleAgent::BuildPath()
{
	if (GetToGoal())
	{
		return;
	}

	//随机浮点数,防止NPC站在相同的点
	const int RANGE = 100;
	float random = (RandomGen::RandUniform(0, RANGE - 1) / (float)RANGE);

    // 防止NPC站在相同的点，尝试在goal_pos周围选择一个可达点
    // 如果在8方向上找不到可达点，就不改变goal_pos
    PointF goal_pos_sel;
    for(int i = 0; i < PF_NEIGHBOR_COUNT; i++)
    {
        goal_pos_sel.x = _goal_pos.x + PF2D_NeighborDist[i * 2    ] * random;
        goal_pos_sel.z = _goal_pos.z + PF2D_NeighborDist[i * 2 + 1] * random;
        if(_movemap->IsPassable(goal_pos_sel)) {
            _goal_pos = goal_pos_sel;
            break;
        }
    }

    std::vector<PF2DVECTOR> path;
    path.push_back(_movemap->TransMap2World(_cur_pos));
    path.push_back(_movemap->TransMap2World(_goal_pos));
    _ramble_path.Init(path);
}

void RambleAgent::SetState(int state)
{
	_agent_state = state;
}

int RambleAgent::GetState() const
{
	return _agent_state;
}

};
