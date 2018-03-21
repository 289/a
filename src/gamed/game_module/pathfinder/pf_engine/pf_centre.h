#ifndef __PF_CENTRE_H__
#define __PF_CENTRE_H__

#include <map>
#include <vector>
#include "point.h"
#include "global_router.h"
#include "shared/base/mutex.h"
#include "shared/base/singleton.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace net
}; // namespace shared

namespace pathfinder
{

class MoveMap;
class PfCentreImp;
struct VexNode;
struct ArcNode;
typedef int32_t MapID;

/**
 * @class PfCentre
 * @brief 寻路中心
 */
class PfCentre : public shared::Singleton<PfCentre>
{
private:
	PfCentreImp* _imp;
	GlobalRouter _router;

public:
	PfCentre();
	virtual ~PfCentre();

	static PfCentre* GetInstance()
	{
		static PfCentre instance;
		return &instance;
	}

	bool Initialize(shared::net::Buffer& buffer);
	bool LoadMoveMap(MapID mapid, shared::net::Buffer& buffer);
	bool FreeMoveMap(MapID mapid);
	bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path);                               //同地图同簇寻路
	bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path);//同地图同簇寻路
	bool SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route);                            //同地图不同簇寻路
	bool SearchRoute(MapID src_mapid, const PointI& source, MapID dest_mapid, const PointI& dest, Route& route);      //跨地图寻路
	void Release();

	MoveMap* QueryMoveMap(MapID mapid);              // 申请通路图(非线程安全)
	MoveMap* AllocWalkMap(MapID mapid);              // 申请通路图(线程安全)
	void FreeWalkMap(MapID mapid, MoveMap* movemap); // 归还通路图(线程安全)

protected:
private:
};


/**
 * @class PfCentreImp
 * @brief 寻路类实体部分
 * @brief 客户端和服务器分别实现
 */
class PfCentreImp
{
protected:
	bool _init;

public:
	PfCentreImp(): _init(false) {}
	virtual ~PfCentreImp() {}

	virtual bool LoadMoveMap(MapID mapid, shared::net::Buffer& buffer) = 0;
	virtual bool FreeMoveMap(MapID mapid) = 0;
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path) = 0;
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path) = 0;
	virtual bool SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route) = 0;
	virtual MoveMap* QueryMoveMap(MapID mapid) = 0;
	virtual MoveMap* AllocWalkMap(MapID mapid) = 0;
	virtual void FreeWalkMap(MapID mapid, MoveMap* movemap) = 0;
	virtual void Release() = 0;
};

class PfCentreImpSvr : public PfCentreImp
{
    static const int kDefaultThreadCount = 5;
private:
	typedef std::vector<MoveMap*> MoveMapVec;
	typedef std::map<MapID, MoveMapVec> MoveMapPool;

	MoveMapPool       _map_pool;
	shared::MutexLock _map_lock;

public:
	PfCentreImpSvr();
	virtual ~PfCentreImpSvr();

	virtual bool LoadMoveMap(MapID mapid, shared::net::Buffer& buffer);
	virtual bool FreeMoveMap(MapID mapid);
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path);
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path);
	virtual bool SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route);
	virtual MoveMap* QueryMoveMap(MapID mapid);
	virtual MoveMap* AllocWalkMap(MapID mapid);
	virtual void FreeWalkMap(MapID mapid, MoveMap* movemap);
	virtual void Release();
};

class PfCentreImpClient : public PfCentreImp
{
protected:
private:
	typedef std::vector<MoveMap*> MoveMapVec;
	MoveMapVec _map_list;
	
public:
	PfCentreImpClient();
	virtual ~PfCentreImpClient();

	virtual bool LoadMoveMap(MapID mapid, shared::net::Buffer& buffer);
	virtual bool FreeMoveMap(MapID mapid);
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, Path& path);
	virtual bool SearchPath(MapID mapid, const PointI& source, const PointI& dest, float step, std::vector<PF2DVECTOR>& path);
	virtual bool SearchRoute(MapID mapid, const PointI& source, const PointI& dest, Route& route);
	virtual MoveMap* QueryMoveMap(MapID mapid);
	virtual MoveMap* AllocWalkMap(MapID mapid);
	virtual void FreeWalkMap(MapID mapid, MoveMap* movemap);
	virtual void Release();
};

};

#endif // __PF_CENTRE_H__
