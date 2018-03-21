#include "chase_agent.h"
#include "move_map.h"
#include "pf_common.h"
#include "pf_centre.h"

namespace pathfinder
{

ChaseAgent::ChaseAgent() :
	_movemap(NULL),
	_range_square(0.0f),
	_agent_state(PF_AGENT_STATE_INIT)
{
}

ChaseAgent::~ChaseAgent()
{
}

void ChaseAgent::Init(int mapid, const PF2DVECTOR& vcurpos)
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
		SetState(PF_AGENT_STATE_UNKNOWN);
		return;
	}
	_cur_pos = pt;
	*/

	_cur_pos = _movemap->TransWorld2Map(vcurpos);
	_vcur_pos = vcurpos;
	SetState(PF_AGENT_STATE_READY);
}

void ChaseAgent::SetGoal(const PF2DVECTOR& vgoal, float distance)
{
	PointF pt = _movemap->TransWorld2Map(vgoal);
	if (!_movemap->IsPassable(pt))
	{
		SetState(PF_AGENT_STATE_UNKNOWN);
		return;
	}

	_goal_pos     = pt;
    _vgoal_pos    = vgoal;
	_range_square = distance * distance;
}

bool ChaseAgent::StartChase()
{
	Path __path;
	if (!PfCentre::GetInstance()->SearchPath(_movemap->GetMapID(), POINTF_2_POINTI(_cur_pos), POINTF_2_POINTI(_goal_pos), __path))
	{
		return false;
	}

	_path.Clear();
	_path.Init(__path);
	return true;
}

bool ChaseAgent::StepMove(float step)
{
	if (_agent_state != PF_AGENT_STATE_READY)
	{
		return false;
	}

	if (TouchGoal())
	{
		return true;
	}

	if (_path.GetLength() <= 1)
	{
		return true;
	}

	PointF next;
	if (_path.GetNextPos(next, (step/_movemap->GetGridSize())) <= 0.0f)
	{
		return false;
	}

	if (_path.GetLength() > 1)
	{
		_cur_pos = next;
	}
	else
	{
		_cur_pos = _goal_pos;
	}

	return true;
}

bool ChaseAgent::TouchGoal() const
{
	if (_agent_state != PF_AGENT_STATE_READY)
	{
		return true;
	}

	float tolerance = 0.1f;
	PF2DVECTOR vCurPos  = _movemap->TransMap2World(_cur_pos);
	PF2DVECTOR vGoalPos = _movemap->TransMap2World(_goal_pos);
	PF2DVECTOR vOffset  = vGoalPos - vCurPos;

	return vOffset.SquaredMagnitude() < (_range_square + tolerance);
}

PF2DVECTOR ChaseAgent::GetCurPos() const
{
	return _movemap->TransMap2World(_cur_pos);
}

PF2DVECTOR ChaseAgent::GetTarget() const
{
	return _vgoal_pos;
}

void ChaseAgent::SetState(int state)
{
	_agent_state = state;
}

int ChaseAgent::GetState() const
{
	return _agent_state;
}

};
