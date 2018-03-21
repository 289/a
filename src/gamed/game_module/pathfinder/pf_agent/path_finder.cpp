#include <stdio.h>
#include <string.h>
#include "game_stl.h"
#include "path_finder.h"
#include "move_map.h"
#include "bit_map.h"
#include "pf_centre.h"
#include "pf_common.h"
#include "ramble_agent.h"
#include "chase_agent.h"
#include "wayfinder_agent.h"
#include "expander.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/**
 * @func  FullName
 * @brief 创建完整的通路图数据文件路径名
 * @brief 例如movemap_1.gbd, 表示1号地图的通路图数据文件
 */
static std::string FullPath(int mapid, const char* filepath)
{
	char __mapid[20];
	memset(__mapid, 0, sizeof(__mapid) / sizeof(char));
	sprintf(__mapid, "%d", mapid);

	std::string __full_path(filepath);
	__full_path.append("movemap_");
	__full_path.append(__mapid);
	__full_path.append(".gbd");
	return __full_path;
}

/**
 * @func  LoadFile
 * @brief 加载文件内容，将文件内容读到buffer中
 */
static bool LoadFile(const char* file_path, shared::net::Buffer& buffer)
{
	if (!file_path)
	{
		return false;
	}

	FILE* fp = fopen(file_path, "rb");
	if (!fp)
	{
		return false;
	}

	size_t file_size = 0;
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	buffer.EnsureWritableBytes(file_size);

	size_t bytes = fread(buffer.BeginWrite(), file_size, 1, fp);
	if (bytes != 1)
	{
		return false;
	}
	buffer.HasWritten(file_size);

	fclose(fp);
	return true;
}

bool InitGlobalTransTable(const char* file_path)
{
	shared::net::Buffer buffer;
	if (!LoadFile(file_path, buffer))
	{
		return false;
	}

	return PfCentre::GetInstance()->Initialize(buffer);
}

bool LoadMoveMap(int mapid, const char* filepath)
{
	if (mapid <= 0 || filepath == NULL)
	{
		return false;
	}

	std::string full_path;
	full_path = FullPath(mapid, filepath);

	shared::net::Buffer buffer;
	if (!LoadFile(full_path.c_str(), buffer))
	{
		return false;
	}

	return PfCentre::GetInstance()->LoadMoveMap(mapid, buffer);
}

bool InitGlobalTransTable(shared::net::Buffer& buffer)
{
	return PfCentre::GetInstance()->Initialize(buffer);
}

bool LoadMoveMap(int32_t mapid, shared::net::Buffer& buffer)
{
	return PfCentre::GetInstance()->LoadMoveMap(mapid, buffer);
}

bool FreeMoveMap(int mapid)
{
	return PfCentre::GetInstance()->FreeMoveMap(mapid);
}

void Release()
{
	PfCentre::GetInstance()->Release();
}

bool SearchPath(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& dest, float step, std::vector<PF2DVECTOR>& path)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return false;
	}

	PointI __source = POINTF_2_POINTI(movemap->TransWorld2Map(source));
	PointI __dest   = POINTF_2_POINTI(movemap->TransWorld2Map(dest));
	if (!PfCentre::GetInstance()->SearchPath(mapid, __source, __dest, step, path))
	{
		path.clear();
		return false;
	}

	if (__source == (POINTF_2_POINTI)(movemap->TransWorld2Map(path.front())))
	{
		if (!path.empty())
		{
			path.erase(path.begin());
		}
	}

	return true;
}

bool IsWalkable(int mapid, const PF2DVECTOR& pos)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return false;
	}

	PointF __pos = movemap->TransWorld2Map(pos);
	return movemap->IsPassable(__pos);
}

bool SetWalkable(int mapid, const PF2DVECTOR& pos, bool walkable)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return false;
	}

	BitMap* bmap = movemap->GetBitMap();
	PointI __pos = POINTF_2_POINTI(movemap->TransWorld2Map(pos));
	if (!movemap->IsValidPos(__pos))
	{
		return false;
	}
	return bmap->SetDynPassable(__pos, walkable);
}

bool SetWalkable(int mapid, const PF2DVECTOR& pos, WalkableUnit unit, bool walkable)
{
  MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
  if ( ! movemap)
  {
    return false;
  }

  BitMap* bmap = movemap->GetBitMap();
  PointI __pos = POINTF_2_POINTI(movemap->TransWorld2Map(pos));

  static int aRangeTable[][4] = {
    {0,0,0,0},
    {0,0,0,0},
    {-1,-1,1,1},
    {-2,-2,2,2},
  };

  // 这里需要查表，所以要保证这些值的可靠性
  SHARED_STATIC_ASSERT(WalkableUnit_1 == 1);
  SHARED_STATIC_ASSERT(WalkableUnit_Center_3x3 == 2);
  SHARED_STATIC_ASSERT(WalkableUnit_Center_5x5 == 3);

  // 一个点的情况
  if(unit == WalkableUnit_1)
  {
    if (movemap->IsValidPos(__pos)) {
      return bmap->SetDynPassable(&__pos, 1, walkable);
    }
    return false;
  }

  // 多个点的情况
  stl_vector<PointI> points;
  points.reserve(5 * 5);
  int* pRange = aRangeTable[unit];
  PointI p;
  for(int z = pRange[2]; z <= pRange[3]; ++z)
  {
    p.z = __pos.z + z;
    for(int x = pRange[0]; x <= pRange[2]; ++x)
    {
      p.x = __pos.x + x;
      if (movemap->IsValidPos(p)) {
        points.push_back(p);
      }
    }
  }
  return bmap->SetDynPassable(&points.front(), points.size(), walkable);
}

bool LineTo(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& dest)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return false;
	}

	PointF __source = movemap->TransWorld2Map(source);
	PointF __dest   = movemap->TransWorld2Map(dest);
	return movemap->LineTo2(__source, __dest);
}

float GetGridSize(int mapid)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return -1.0f;
	}
	return movemap->GetGridSize();
}

bool FindNearbyWalkablePos(int mapid, const PF2DVECTOR& pos, PF2DVECTOR& newpos)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		return false;
	}

	PointF __pos = movemap->TransWorld2Map(pos);
	if (!movemap->IsValidPos(POINTF_2_POINTI(__pos)))
	{
		return false;
	}

	/*
	if (movemap->IsPassable(__pos))
	{
		newpos = pos;
		return true;
	}
	*/

	PointI __newpos;
	Expander expander;
	if (expander.Search(movemap->GetBitMap(), POINTF_2_POINTI(__pos), __newpos))
	{
		newpos = movemap->TransMap2World(__newpos);
		return true;
	}

	newpos = PF2DVECTOR(-10000.0f,-10000.0f);
	return false;
}

bool IsInSameArea(int mapid, const PF2DVECTOR& pos1, const PF2DVECTOR& pos2)
{
	MoveMap* movemap = PfCentre::GetInstance()->QueryMoveMap(mapid);
	if (!movemap)
	{
		assert(false);
		return false;
	}

	PointF __pos1 = movemap->TransWorld2Map(pos1);
	PointF __pos2 = movemap->TransWorld2Map(pos2);
	if (!movemap->InSameArea(__pos1, __pos2))
	{
		return false;
	}
	return true;
}

/******************************NpcRamble*********************************/
/******************************NpcRamble*********************************/
/******************************NpcRamble*********************************/
/******************************NpcRamble*********************************/

NpcRamble::NpcRamble()
{
	_agent = new RambleAgent();
}

NpcRamble::~NpcRamble()
{
	delete _agent;
	_agent = NULL;
}

bool NpcRamble::Start(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& center, float speed, float range)
{
	_agent->Init(mapid, source);
	_agent->SetRambleInfo(center, range);
	_agent->SetMoveStep(speed);
	if (_agent->GetState() != PF_AGENT_STATE_READY)
	{
		return false;
	}

	_agent->StartRamble();
	return true;
}

bool NpcRamble::MoveOneStep()
{
	return _agent->StepMove();
}

PF2DVECTOR NpcRamble::GetCurPos() const
{
	return _agent->GetCurPos();
}

PF2DVECTOR NpcRamble::GetGoalPos() const
{
	return _agent->GetGoalPos();
}

bool NpcRamble::GetToGoal() const
{
	return _agent->GetToGoal();
}


/******************************NpcChase*********************************/
/******************************NpcChase*********************************/
/******************************NpcChase*********************************/
/******************************NpcChase*********************************/

NpcChase::NpcChase()
{
	_agent = new ChaseAgent();
}

NpcChase::~NpcChase()
{
	delete _agent;
	_agent = NULL;
}

bool NpcChase::Start(int mapid, const PF2DVECTOR& source, const PF2DVECTOR& target, float range)
{
	_agent->Init(mapid, source);
	_agent->SetGoal(target, range);
	if (_agent->GetState() != PF_AGENT_STATE_READY)
	{
		return false;
	}

	_agent->StartChase();
	return true;
}

bool NpcChase::MoveOneStep(float step)
{
	return _agent->StepMove(step);
}

PF2DVECTOR NpcChase::GetCurPos() const
{
	return _agent->GetCurPos();
}

PF2DVECTOR NpcChase::GetTarget() const
{
	return _agent->GetTarget();
}

bool NpcChase::GetToGoal() const
{
	return _agent->TouchGoal();
}

/******************************NpcFixedRamble*********************************/
/******************************NpcFixedRamble*********************************/
/******************************NpcFixedRamble*********************************/
/******************************NpcFixedRamble*********************************/

NpcFixedRamble::NpcFixedRamble():
	_mapid(0), _speed(0), _range(0), _agent(NULL)
{

}

NpcFixedRamble::~NpcFixedRamble()
{
	if (_agent != NULL)
	{
		delete _agent;
		_agent = NULL;
	}
}

bool NpcFixedRamble::Init(int mapid, float speed, float range, const PF2DVECTOR& center)
{
	_mapid = mapid;
	_speed = speed;
	_range = range;
	_center = center;
	GetToGoal();
	return Start(_center);
}

bool NpcFixedRamble::Start(const PF2DVECTOR& source)
{
	_agent->Init(_mapid, source);
	_agent->SetRambleInfo(_center, _range);
	_agent->SetMoveStep(_speed);	
	if (_agent->GetState() != PF_AGENT_STATE_READY)
	{
		return false;
	}

	_agent->StartRamble();
	return true;
}

PF2DVECTOR NpcFixedRamble::GetGoalPos() const
{
	return _agent->GetGoalPos();
}

void NpcFixedRamble::GetToGoal()
{
	if (_agent != NULL)
	{
		delete _agent;
		_agent = NULL;
	}

	_agent = new RambleAgent;
}

/******************************WayFinder*********************************/
/******************************WayFinder*********************************/
/******************************WayFinder*********************************/
/******************************WayFinder*********************************/
WayFinder::WayFinder():
	_agent(NULL)
{
}

WayFinder::~WayFinder()
{
	if (_agent)
	{
		delete _agent;
		_agent = NULL;
	}
}

bool WayFinder::Start(int src_mapid, const PF2DVECTOR& src, int dest_mapid, const PF2DVECTOR& dest, float step)
{
	if (_agent)
	{
		delete _agent;
		_agent = NULL;
	}

	_agent = new WayFinderAgent();

	_agent->Init(src_mapid, src, dest_mapid, dest);
	_agent->SetMoveStep(step);
	if (_agent->GetState() != PF_AGENT_STATE_READY)
	{
		return false;
	}

	return _agent->Start();
}

bool WayFinder::NextPath(std::vector<PF2DVECTOR>& path)
{
	if (_agent->GetState() != PF_AGENT_STATE_READY)
	{
		return false;
	}

	if (!_agent->NextPath(path))
	{
		path.clear();
		return false;
	}

	if (!path.empty())
	{
		path.erase(path.begin());
	}

	return true;
}

bool WayFinder::GetCurStartPos(PF2DVECTOR& pos)
{
	return _agent->GetCurStartPos(pos);
}

bool WayFinder::GetCurEndPos(PF2DVECTOR& pos)
{
	return _agent->GetCurEndPos(pos);
}

};
