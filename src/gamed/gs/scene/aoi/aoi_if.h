#ifndef GAMED_GS_SCENE_AOI_IF_H_
#define GAMED_GS_SCENE_AOI_IF_H_

#include <vector>
#include <map>
#include <set>

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "utility_lib/CGLib/cglib.h"
#include "gs/global/math_types.h"


namespace gamed {

class AoiPlane;
class AoiSlice;

/**
 * @brief 新版AOI，使用打格子方式控制
 */
class AOI : shared::noncopyable
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
	bool    Init(const rect& scope_rt, float step, float near, float far, AOI_Callback aoi_cb, void* cb_data);
	
	// ---- thread safe ----
	// （1）插入一个可以移动的物体
	// （2）mode: w(atcher) m(arker)
	bool    InsertDynamicObj(ObjectID id,
			                 IDType type,
							 int64_t param,
			                 const char* mode,
			                 const A2DVECTOR& init_pos,
			                 bool area_detect);
	bool    DropDynamicObj(ObjectID id);

	// ---- thread safe ----
	//  （1）area在AOI只是一个watcher，其他对象无法观察到它，因此通知对象进入、离开该区域
	//       只能放在area的上层逻辑进行，即/obj/area.h
	bool    InsertPolygonArea(ObjectID id,
			                  IDType type,
			                  const PolygonPointsVec& points);
	bool    DropPolygonArea(ObjectID id);

	// ---- thread safe ----
	// 只能由Object本身update自己的位置
	void    AOI_Update(ObjectID id, const A2DVECTOR& new_pos);

	// **** thread safe ****
	// 只能在world的心跳里调用
	void    AOI_Heartbeat();


public:
	struct object
	{
		ObjectID  id;
		IDType    type;
		int       mode;
		A2DVECTOR position;
		bool      area_detect;
		int64_t   param;
	};


protected:
	struct polygon_area
	{
		ObjectID         id;
		IDType           type;
		bool             is_convex;
		CGLib::Polygon   polygon;
		CGLib::Rectangle rect; // 多边形区域的aabb外接矩形，用于性能优化（减少判断点是否在多边形内）
	};

	struct between_slice
	{
		between_slice()
			: is_cross_slice(false),
			  pPiece(NULL),
			  pNewPiece(NULL)
		{ }

		bool      is_cross_slice;
		AoiSlice* pPiece;
		AoiSlice* pNewPiece;
	};

	typedef std::map<ObjectID, object*> IDToObjectMap;
	typedef std::map<ObjectID, polygon_area*> IDToAreaMap;

	// **** thread unsafe ****
	// （1）除aoi中所有对象，但是不释放plane，网格不变.
	// （2）AOI需要回收时必须先调该函数
	void    Clear();

	
private:
	object* NewObject(ObjectID id, int mode, IDType type, int64_t param, const A2DVECTOR& pos, bool area_detect);
	polygon_area* NewPolygonArea(ObjectID id, IDType type, const PolygonPointsVec& points);
	between_slice MoveBetweenSlice(const object* obj, const A2DVECTOR& new_pos);

	void    AutoBroadcastCSMsg(const object& obj, AOI_Type aoitype);
	void    InsertObjToMap(object* obj);
	void    DropObjectFromMap(object* obj);
	void    InsertAreaToMap(polygon_area* area);
	inline void DeleteObject(object* obj);
	inline void DeleteArea(polygon_area* area);

	// area func
	void    AreaDoChange();
	void    AreaDetection(const object* obj, const A2DVECTOR& pos, bool obj_is_drop);
	void    ObjectDropFromArea(const object* obj);
	bool    CheckPointInArea(const polygon_area* parea, const A2DVECTOR& pos) const;
	void    EnterAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data);
	void    LeaveAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data);


private:
	rect           scope_rect_;
	AoiPlane*      plane_;

	///
	/// 以下成员变量，Clear()时需要清除
	///
	AOI_Callback   aoi_cb_;
	void*          cb_data_;

	// obj map
	IDToObjectMap  object_map_;

	// 需要lock两个mutex时，锁的顺序是：(1)mutex_obj_map_ (2)mutex_area_map_
	shared::MutexLock mutex_obj_map_;
	shared::MutexLock mutex_area_map_;

	// polygon area
	IDToAreaMap      area_map_;
	IDToAreaMap      waiting_area_map_;
	typedef std::set<ObjectID> ObjectIDSet;
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

} // namespace gamed

#endif // GAMED_GS_SCENE_AOI_IF_H_
