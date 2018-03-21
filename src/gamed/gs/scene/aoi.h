#ifndef GAMED_GS_SCENE_AOI_H_
#define GAMED_GS_SCENE_AOI_H_

#include <stdint.h>
#include <set>
#include <list>
#include <map>
#include <vector>

#include "shared/base/mutex.h"
#include "utility_lib/CGLib/cglib.h"
#include "gs/global/math_types.h"


namespace gamed {

using namespace shared;

#define DIST2(p1, p2) ((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y))

class A2DVECTOR;

/**
 * @brief 
 *  （1）对象分为两类，一类叫观察者 Watcher ，另一类是被观察者 Marker, Drop表示丢弃掉这个对象
 *  （2）AOI模块可以单独测试，见当前目录下的tests/
 *  （3）AOI模块只管理最大范围的移动视野，仇恨视野由业务逻辑层自行管理。
 *       因此，AOI里所有Object的视野是固定的圆形，区域视野是多边形，而且只能作为观察者。
 *  （4）目前AOI模块只有在执行"d" mode的时候会主动通知所有观察者，即object leave view。
 *       mode从"wm"变为"w"不会主动通知。(2013-06-05)
 */
class AOI
{
	typedef int64_t  ObjectID;
	typedef uint16_t IDType;
	typedef std::vector<CGLib::Point2d> PolygonPointsVec;
public:
	enum AOI_Type
	{
		OBJ_NOT_DEF,
		OBJ_ENTER,
		OBJ_LEAVE,
	};

	struct ObjectInfo
	{
		ObjectID id;
		IDType   type;
		int64_t  param;
	};

	typedef void (*AOI_Callback)(AOI_Type type, ObjectInfo watcher, ObjectInfo marker, void* cb_data);

	AOI();
	~AOI();

	// **** thread unsafe ****
	inline void SetObjectFieldOfView(float rad_short, float rad_long);	

	// ---- thread safe ----
	// （1）环形视野Object查询半径是一个条带,这样做是为了防止结果颠簸，
	//      WorldObject都是AnnulusObj（包括player, npc等）
	// （2）这个函数只承诺AOI会返回进入rad_short半径内的对象，以及会会返回rad_long的对象离开
	bool    InsertAnnulusObj(ObjectID id, 
			                 IDType type,
							 int64_t param,
			                 const char* mode, 
							 const A2DVECTOR& init_pos,
							 bool area_detect);

	// ---- thread safe ----
	//  （1）多边形区域，area并没有参与AOI的心跳没有加入hot_pair，是独立的
	//  （2）area在AOI只是一个watcher，其他对象无法观察到它，因此通知对象进入、离开该区域
	//       只能放在area的上层逻辑进行，即/obj/area.h
	bool    InsertPolygonArea(ObjectID id,
			                  IDType type,
							  const PolygonPointsVec& points);
	bool    DropPolygonArea(ObjectID id);
	
	// **** thread unsafe ****
	// 只能由Object本身update自己的位置
	// w(atcher) m(arker) d(rop)
	void    AOI_Update(ObjectID id, const char* mode, const A2DVECTOR& pos);

	// **** thread unsafe ****
	// 只能在world的心跳里调用
	void    AOI_Message(AOI_Callback aoi_cb, void* cb_data = NULL);


private:
	struct polygon_area
	{
		ObjectID         id;
		IDType           type;
		bool             is_convex;
		CGLib::Polygon   polygon;
		CGLib::Rectangle rect; // 多边形区域的aabb外接矩形，用于性能优化（减少判断点是否在多边形内）
	};

	struct object
	{
		ObjectID  id;
		IDType    type;
		int       version;
		int       mode;
		A2DVECTOR last;
		A2DVECTOR position;
		bool      area_detect;
		int64_t   param;
	};
	typedef std::set<object*> ObjectSet;

	struct hot_pair
	{
		struct object* watcher;
		struct object* marker;
		int watcher_version;
		int marker_version;
	};
	typedef std::list<hot_pair> PairList;

	typedef std::map<ObjectID, object*> IDToObjectMap;
	typedef std::map<ObjectID, polygon_area*> IDToAreaMap;

	object*        NewObject(ObjectID id, int mode, IDType type, int64_t param, const A2DVECTOR& last, const A2DVECTOR& pos, bool area_detect);
	polygon_area*  NewPolygonArea(ObjectID id, IDType type, const PolygonPointsVec& points);
	void           InsertObjToMap(object* obj);
	void           InsertAreaToMap(polygon_area* area);
	void           DropObject(object* obj);
	inline void    DeleteObject(object* obj);
	inline void    DeleteArea(polygon_area* area);
	inline void    ClearObjectSet(ObjectSet& obj_set);

	inline bool    is_near(A2DVECTOR p1, A2DVECTOR p2);
	inline float   dist2(object* p1, object* p2);

	// For thread-safe: The following four functions can just run in AOI_Message()
	void    EnterViewCB(AOI_Callback aoi_cb, const object* watcher, const object* marker, void* cb_data);
	void    LeaveViewCB(AOI_Callback aoi_cb, const object* watcher, const object* marker, void* cb_data);
	void    EnterAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data);
	void    LeaveAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data);

	// area func
	void    AreaDoChange();
	void    AreaDetection(const object* obj, const A2DVECTOR& pos, bool obj_is_drop);
	void    ObjectDropFromArea(const object* obj);
	bool    CheckPointInArea(const polygon_area* parea, const A2DVECTOR& pos) const;
	
	bool    ChangeMode(object* obj, bool set_watcher, bool set_marker);
	void    DropPair(hot_pair* pair);
	void    FlushPair();
	void    SetPush(object* obj);
	void    GenHotPair(object* watcher, object* marker);
    void    GenHotPairList(ObjectSet& watcher, ObjectSet& marker);

	void    PackObjectInfo(const object* obj, ObjectInfo& info);
	void    PackAreaObjectInfo(const polygon_area* area, ObjectInfo& info);


private:
	IDToObjectMap  object_map_;
	ObjectSet      watcher_static_;
	ObjectSet      marker_static_;
	ObjectSet      watcher_move_;
	ObjectSet      marker_move_;
	PairList       hot_list_;

	typedef std::set<ObjectID> ObjectIDSet;
	typedef std::map<ObjectID, ObjectIDSet> ObjHotPairSaveMap;
	ObjHotPairSaveMap  watcher_hotpair_savemap_;
	ObjHotPairSaveMap  marker_hotpair_savemap_;

	float          rad_short_;
	float          rad_long_;

	// 需要lock两个mutex时，锁的顺序是：(1)mutex_obj_map_ (2)mutex_area_map_
	MutexLock      mutex_obj_map_;
	MutexLock      mutex_area_map_;

	AOI_Callback   aoi_cb_;
	void*          cb_data_;

	// polygon area
	IDToAreaMap      area_map_;
	IDToAreaMap      waiting_area_map_;
	typedef std::map<ObjectID, ObjectIDSet> AreaViewSaveMap;
	AreaViewSaveMap  area_mapto_objset_; // area内的objects
	AreaViewSaveMap  obj_mapto_areaset_; // object所在的areas
};

///
/// inline func
///
inline void AOI::DeleteObject(object* obj)
{
	delete obj;
}

inline void AOI::DeleteArea(polygon_area* area)
{
	delete area;
}

inline bool AOI::is_near(A2DVECTOR p1, A2DVECTOR p2)
{
	return DIST2(p1, p2) < rad_short_ * rad_short_ * 0.25f;
}

inline float AOI::dist2(object* p1, object* p2)
{
	float d = DIST2(p1->position, p2->position);
	return d;
}

inline void AOI::SetObjectFieldOfView(float rad_short, float rad_long)
{
	rad_short_ = rad_short;
	rad_long_  = rad_long;
}

inline void AOI::ClearObjectSet(ObjectSet& obj_set)
{
	obj_set.clear();
}

} // namespace gamed

#endif // GAMED_GS_SCENE_AOI_H_
