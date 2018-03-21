#include <string.h>
#include "game_stl.h"
#include "shared/security/randomgen.h"

#include "pf_centre.h"
#include "move_map.h"
#include "graph_astar.h"
#include "dgraph.h"
#include "pf_engine.h"
#include "pf_common.h"
#include "point.h"

namespace pathfinder
{

/********************************PfCentre********************************/
/********************************PfCentre********************************/
/********************************PfCentre********************************/
/********************************PfCentre********************************/

using namespace shared::net;

PfCentre::PfCentre():
	_imp(NULL)
{
}

PfCentre::~PfCentre()
{
	assert(!_imp);
}

bool PfCentre::Initialize(shared::net::Buffer& buffer)
{
#ifdef CLIENT_SIDE
	_imp = new PfCentreImpClient();
	if (!_imp)
	{
		return false;
	}
#else
	_imp = new PfCentreImpSvr();
	if (!_imp)
	{
		return false;
	}
#endif

	RandomGen::Init();

	//初始化全局传送图数据
	if (!_router.Init(buffer))
	{
		return false;
	}
	return true;
}

void PfCentre::Release()
{
	if (_imp)
	{
		_imp->Release();
		delete _imp;
		_imp = NULL;
	}

	_router.Release();
}

bool PfCentre::LoadMoveMap(MapID mapid, shared::net::Buffer& buffer)
{
	return _imp->LoadMoveMap(mapid, buffer);
}

bool PfCentre::FreeMoveMap(MapID mapid)
{
	return _imp->FreeMoveMap(mapid);
}

MoveMap* PfCentre::QueryMoveMap(MapID mapid)
{
	return _imp->QueryMoveMap(mapid);
}

MoveMap* PfCentre::AllocWalkMap(MapID mapid)
{
	return _imp->AllocWalkMap(mapid);
}

void PfCentre::FreeWalkMap(MapID mapid, MoveMap* movemap)
{
	_imp->FreeWalkMap(mapid, movemap);
}

bool PfCentre::SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path)
{
	return _imp->SearchPath(mapid, source, dest, step, path);
}

bool PfCentre::SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path)
{
	return _imp->SearchPath(mapid, source, dest, path);
}

bool PfCentre::SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route)
{
	return _imp->SearchRoute(mapid, source, dest, route);
}

bool PfCentre::SearchRoute(MapID src_mapid, const PointI& source, MapID dst_mapid, const PointI& dest, Route& route)
{
	return _router.SearchRoute(src_mapid, source, dst_mapid, dest, route);
}

/********************************PfCentreImpSvr********************************/
/********************************PfCentreImpSvr********************************/
/********************************PfCentreImpSvr********************************/
/********************************PfCentreImpSvr********************************/

#define ASSERT_INIT(init) if(!init) {assert(init);}

PfCentreImpSvr::PfCentreImpSvr(): PfCentreImp()
{
}

PfCentreImpSvr::~PfCentreImpSvr()
{
	assert(_map_pool.empty());
}

bool PfCentreImpSvr::LoadMoveMap(MapID mapid, shared::net::Buffer& buffer)
{
    for (int i = 0; i < kDefaultThreadCount; ++i) 
    {
        shared::net::Buffer tmpbuf = buffer;
        MoveMap* movemap = new MoveMap();
        if (!movemap)
        {
            return false;
        }

        if (!movemap->Load(tmpbuf) || movemap->GetMapID() != mapid)
        {
            assert(false);
            delete movemap;
            return false;
        }

        _map_lock.lock();
        _map_pool[mapid].push_back(movemap);
        _init = true;
        _map_lock.unlock();
    }

	//test
	//movemap->dump();
	return true;
}

bool PfCentreImpSvr::FreeMoveMap(MapID mapid)
{
	assert(false);
	return false;
}

bool PfCentreImpSvr::SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path)
{
	ASSERT_INIT(_init);

	MoveMap* movemap = AllocWalkMap(mapid);
	if (!movemap)
	{
		//assert(false);
		return false;
	}

	Path __path;
	PfEngine engine(movemap);
	engine.Setup(source, dest);
	if (engine.Search(__path) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		FreeWalkMap(mapid, movemap);
		return false;
	}

	for (size_t i = 0; i < __path.size(); ++ i)
	{
		PF2DVECTOR __tmp = movemap->TransMap2World(__path[i]);
		path.push_back(__tmp);
	}

	engine.Cleanup();
	FreeWalkMap(mapid, movemap);
	return true;
}

bool PfCentreImpSvr::SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path)
{
	ASSERT_INIT(_init);

	MoveMap* movemap = AllocWalkMap(mapid);
	if (!movemap)
	{
		//assert(false);
		return false;
	}

	PfEngine engine(movemap);
	engine.Setup(source, dest);
	if (engine.Search(path) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		FreeWalkMap(mapid, movemap);
		return false;
	}

	engine.Cleanup();
	FreeWalkMap(mapid, movemap);
	return true;
}

bool PfCentreImpSvr::SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route)
{
	ASSERT_INIT(_init);

	MoveMap* movemap = AllocWalkMap(mapid);
	if (!movemap)
	{
		//assert(false);
		return false;
	}

	route.clear();
	PfEngine engine(movemap);
	engine.Setup(source, dest);
	if (engine.Search(route) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		FreeWalkMap(mapid, movemap);
		return false;
	}
	engine.Cleanup();

	FreeWalkMap(mapid, movemap);
	return true;
}

void PfCentreImpSvr::Release()
{
	shared::MutexLockGuard lock(_map_lock);

	MoveMapPool::iterator it = _map_pool.begin();
	for (; it != _map_pool.end(); ++ it)
	{
		MoveMapVec& list = it->second;
		for (size_t i = 0; i < list.size(); ++ i)
		{
			delete list[i];
			list[i] = NULL;
		}
		list.clear();
	}
	_map_pool.clear();
	_init = false;
}

MoveMap* PfCentreImpSvr::QueryMoveMap(MapID mapid)
{
	/**
	 * 线程不安全
	 */

	ASSERT_INIT(_init);

	MoveMapPool::iterator it = _map_pool.find(mapid);
	if (it == _map_pool.end())
	{
		assert(false);
		return NULL;
	}

	return it->second.front();
}

MoveMap* PfCentreImpSvr::AllocWalkMap(MapID mapid)
{
	ASSERT_INIT(_init);
	shared::MutexLockGuard lock(_map_lock);

	MoveMapPool::iterator it = _map_pool.find(mapid);
	if (it == _map_pool.end())
	{
		assert(false);
		return NULL;
	}

	for (size_t i = 0; i < it->second.size(); ++ i)
	{
		MoveMap* movemap = it->second[i];
		if (movemap->TestActive()) continue;
		movemap->SetActive();
		return movemap;
	}

	//通路图数量小于线程数,部分线程请求得不到满足
    assert(false);
	return NULL;
}

void PfCentreImpSvr::FreeWalkMap(MapID mapid, MoveMap* movemap)
{
	ASSERT_INIT(_init);
	assert(movemap->TestActive());

	shared::MutexLockGuard lock(_map_lock);

	MoveMapPool::iterator it = _map_pool.find(mapid);
	assert(it != _map_pool.end());
	for (size_t i = 0; i < it->second.size(); ++ i)
	{
		if (it->second[i] == movemap)
		{
			movemap->ClrActive();
			return;
		}
	}

	assert(false);
}

/********************************PfCentreImpClient********************************/
/********************************PfCentreImpClient********************************/
/********************************PfCentreImpClient********************************/
/********************************PfCentreImpClient********************************/

PfCentreImpClient::PfCentreImpClient(): PfCentreImp()
{
}

PfCentreImpClient::~PfCentreImpClient()
{
	assert(_map_list.empty());
}

struct MapFinder
{
	int32_t mapid;
	MapFinder(int32_t __mapid): mapid(__mapid) {}
	bool operator () (MoveMap* movemap) const
	{
		return mapid == movemap->GetMapID();
	}
};

bool PfCentreImpClient::LoadMoveMap(MapID mapid, shared::net::Buffer& buffer)
{
	MoveMapVec::iterator it = std::find_if(_map_list.begin(), _map_list.end(), MapFinder(mapid));
	assert(it == _map_list.end());

	MoveMap* __movemap = new MoveMap();
	if (!__movemap)
	{
		return false;
	}

	if (!__movemap->Load(buffer) || __movemap->GetMapID() != mapid)
	{
		assert(false);
		delete __movemap;
		return false;
	}

	__movemap->SetActive();
	_map_list.push_back(__movemap);
	assert(_map_list.size() <= 2);

	_init = true;
	return true;
}

bool PfCentreImpClient::FreeMoveMap(MapID mapid)
{
	MoveMapVec::iterator it = std::find_if(_map_list.begin(), _map_list.end(), MapFinder(mapid));
	assert(it != _map_list.end());

	delete *it;
	_map_list.erase(it);
	return true;
}

bool PfCentreImpClient::SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path)
{
	ASSERT_INIT(_init);

	MoveMap* movemap = AllocWalkMap(mapid);
	if (!movemap)
	{
		//assert(false);
		return false;
	}

	Path __path;
	PfEngine engine(movemap);
	engine.Setup(source, dest);
	if (engine.Search(__path) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		FreeWalkMap(mapid, movemap);
		return false;
	}

	for (size_t i = 0; i < __path.size(); ++ i)
	{
		PointF __tmp = movemap->TransMap2World(__path[i]);
		path.push_back(__tmp);
	}

	engine.Cleanup();
	FreeWalkMap(mapid, movemap);
	return true;
}

bool PfCentreImpClient::SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path)
{
	ASSERT_INIT(_init);
	MoveMap* __movemap = AllocWalkMap(mapid);
	if (!__movemap)
	{
		return false;
	}

	PfEngine engine(__movemap);
	engine.Setup(source, dest);
	if (engine.Search(path) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		return false;
	}

	engine.Cleanup();
	return true;
}

bool PfCentreImpClient::SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route)
{
	ASSERT_INIT(_init);
	MoveMap* __movemap = AllocWalkMap(mapid);
	if (!__movemap)
	{
		return false;
	}

	route.clear();
	PfEngine engine(__movemap);
	engine.Setup(source, dest);
	if (engine.Search(route) != PF_ENGINE_STATE_FOUND)
	{
		engine.Cleanup();
		return false;
	}

	engine.Cleanup();
	return true;
}

void PfCentreImpClient::Release()
{
	for (size_t i = 0; i < _map_list.size(); ++ i)
		delete _map_list[i];

	_map_list.clear();
}

MoveMap* PfCentreImpClient::QueryMoveMap(MapID mapid)
{
	ASSERT_INIT(_init);
	MoveMapVec::iterator it = std::find_if(_map_list.begin(), _map_list.end(), MapFinder(mapid));
	return it != _map_list.end() ? *it : NULL;
}

MoveMap* PfCentreImpClient::AllocWalkMap(MapID mapid)
{
	ASSERT_INIT(_init);
	MoveMapVec::iterator it = std::find_if(_map_list.begin(), _map_list.end(), MapFinder(mapid));
	return it != _map_list.end() ? *it : NULL;
}

void PfCentreImpClient::FreeWalkMap(MapID mapid, MoveMap* movemap)
{
	ASSERT_INIT(_init);
	//do nothing
}

};
