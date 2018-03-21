#include "wayfinder_agent.h"
#include "pf_common.h"
#include "path_track.h"
#include "move_map.h"
#include "pf_centre.h"
#include "clock.h"

namespace pathfinder
{

/**********************************WayFinderAgent**********************************/
/**********************************WayFinderAgent**********************************/
/**********************************WayFinderAgent**********************************/
/**********************************WayFinderAgent**********************************/

WayFinderAgent::WayFinderAgent() :
	_move_step(0.0f),
	_src_mapid(0),
	_dest_mapid(0),
	_cur_map(-1),
	_cur_pos(-1),
	_state(PF_AGENT_STATE_INIT)
{
}

WayFinderAgent::~WayFinderAgent()
{
	_cur_pos = -1;
	_cur_map = -1;

  CleanupRoute();
}

void WayFinderAgent::Init(int src_mapid, const PF2DVECTOR& source, int dest_mapid, const PF2DVECTOR& dest)
{
	MoveMap* src_movemap  = PfCentre::GetInstance()->QueryMoveMap(src_mapid);
	MoveMap* dest_movemap = PfCentre::GetInstance()->QueryMoveMap(dest_mapid);
	if (!src_movemap || !dest_movemap)
	{
		_state = PF_AGENT_STATE_INVALID_MAP;
		return;
	}

	_vsource = source;
	_vdest   = dest;
	_source  = POINTF_2_POINTI(src_movemap->TransWorld2Map(_vsource));
	_dest    = POINTF_2_POINTI(dest_movemap->TransWorld2Map(_vdest));
	if (!src_movemap->IsValidPos(_source) || !dest_movemap->IsValidPos(_dest))
	{
		_state = PF_AGENT_STATE_INVALID_POS;
		return;
	}

	if (!src_movemap->IsPassable(_source))
	{
		_state = PF_AGENT_STATE_INVALID_POS;
		return;
	}

	_src_mapid   = src_mapid;
	_dest_mapid  = dest_mapid;
	_state = PF_AGENT_STATE_READY;
}

void WayFinderAgent::SetMoveStep(float move_step)
{
	_move_step = move_step;
}

bool WayFinderAgent::Start()
{
	Route  __local_route;
	Route  __map_route;
	PointI __source = _source;
	PointI __dest   = _dest;

  CleanupRoute();
	///
	///地图内寻路
	///
	if (_src_mapid == _dest_mapid)
	{
		if (!PfCentre::GetInstance()->SearchRoute(_src_mapid, __source, __dest, __local_route))
		{
			return false;
		}

		_cur_map = 0;
		_cur_pos = 0;
		_cur_route = __local_route;
		return true;
	}

	Clock clock("wayfinder");;
	clock.Start();

	///
	///跨地图寻路
	///
	if (!PfCentre::GetInstance()->SearchRoute(_src_mapid, __source, _dest_mapid, __dest, __map_route))
	{
		clock.End();
		clock.ElapseTime();
		_state = PF_AGENT_STATE_CLOSE;
		return false;
	}

	assert(__map_route.size() >= 4);
	assert(__map_route.size() % 2 == 0);

	//确定本地图路由
	PointI __start = __map_route[0]->GetCoord();
	PointI __goal  = __map_route[1]->GetCoord();
	if (!PfCentre::GetInstance()->SearchRoute(_src_mapid, __start, __goal, __local_route))
	{
		clock.End();
		clock.ElapseTime();
		_state = PF_AGENT_STATE_CLOSE;
		return false;
	}

	clock.End();
	clock.ElapseTime();

	_cur_map = 0;
	_cur_pos = 0;
	_map_route = __map_route;
	_cur_route = __local_route;

	return true;
}

bool WayFinderAgent::NextPath(std::vector<PF2DVECTOR>& path)
{
	if (_state == PF_AGENT_STATE_CLOSE)
	{
		return false;
	}

	assert(_cur_pos < (_cur_route.size() -1));
	assert(_cur_route[_cur_pos]->GetOwner() == _cur_route[_cur_pos+1]->GetOwner());

	//1)确定目标地图、起点、终点
	PointI __source = _cur_route[_cur_pos]->GetCoord();
	PointI __dest   = _cur_route[_cur_pos+1]->GetCoord();
	MapID  __mapid  = _src_mapid;

	if (!_map_route.empty())
	{
		__mapid = _map_route[_cur_map]->GetOwner();
	}

	//2)在目标地图上寻路
	if (!PfCentre::GetInstance()->SearchPath(__mapid, __source, __dest, _move_step, path))
	{
		_state = PF_AGENT_STATE_CLOSE;
		return false;
	}

	//3)删除所得路径的起点
	MoveMap* __movemap = PfCentre::GetInstance()->QueryMoveMap(__mapid);
	if (__source == (POINTF_2_POINTI)(__movemap->TransWorld2Map(path.front())))
	{
		if (!path.empty())
		{
			path.erase(path.begin());
		}
	}

	//4)更新位置信息
	UpdateCursor();
	return true;
}

bool WayFinderAgent::UpdateCursor()
{
	_cur_pos += 2;
	if (_cur_pos  >= _cur_route.size())
	{
		//本地图寻路结束
		_cur_pos = 0;

		delete _cur_route.front();
		delete _cur_route.back();
        _cur_route.clear();

		//切换地图
		_cur_map += 2;
		if (_cur_map >= _map_route.size())
		{
			//寻路结束
			_cur_pos = -1;
			_cur_map = -1;
			_state = PF_AGENT_STATE_CLOSE;
			return true;
		}
		else
		{
			//跨地图,确定新地图内路由
			int32_t src_mapid  = _map_route[_cur_map]->GetOwner();
			int32_t dest_mapid = _map_route[_cur_map+1]->GetOwner();
			assert(src_mapid == dest_mapid);

			PointI __source = _map_route[_cur_map]->GetCoord();
			PointI __dest   = _map_route[_cur_map+1]->GetCoord();

			Route  __route;
			if (!PfCentre::GetInstance()->SearchRoute(src_mapid, __source, __dest, __route))
			{
				_cur_pos = -1;
				_cur_map = -1;
				_state = PF_AGENT_STATE_CLOSE;
				return false;
			}
			_cur_route = __route;
		}
	}

	return true;
}

bool WayFinderAgent::GetCurStartPos(PF2DVECTOR& pos)
{
	if (GetState() == PF_AGENT_STATE_CLOSE)
	{
		return false;
	}

	int32_t mapid = _src_mapid;
	if (!_map_route.empty())
	{
		mapid = _map_route[_cur_map]->GetOwner();
	}

	MoveMap* movemap  = PfCentre::GetInstance()->QueryMoveMap(mapid);
	assert(movemap);

	pos = movemap->TransMap2World(_cur_route[_cur_pos]->GetCoord());
	return true;
}

bool WayFinderAgent::GetCurEndPos(PF2DVECTOR& pos)
{
	if (GetState() == PF_AGENT_STATE_CLOSE)
	{
		return false;
	}

	int32_t mapid = _src_mapid;
	if (!_map_route.empty())
	{
		mapid = _map_route[_cur_map]->GetOwner();
	}

	MoveMap* movemap  = PfCentre::GetInstance()->QueryMoveMap(mapid);
	assert(movemap);

	pos = movemap->TransMap2World(_cur_route[_cur_pos+1]->GetCoord());
	return true;
}

int WayFinderAgent::GetState() const
{
	return _state;
}

void WayFinderAgent::CleanupRoute()
{
  if (!_map_route.empty())
  {
    delete _map_route.front();
    delete _map_route.back();
  }

  if (!_cur_route.empty())
  {
    delete _cur_route.front();
    delete _cur_route.back();
  }

  _map_route.clear();
  _cur_route.clear();

}

};
