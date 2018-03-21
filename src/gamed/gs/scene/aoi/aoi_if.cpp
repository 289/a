#include "aoi_if.h"

#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"

#include "aoi_plane.h"


namespace gamed {

using namespace shared;

namespace {
	
	static const float COMPLEMENT_EPSINON = (1 - STD_EPSINON);

	enum Mode_Def
	{
		MODE_WATCHER = 0x0001,
		MODE_MARKER  = 0x0002,
	};

	void slice_event(AOI::AOI_Type aoitype, const AOI::object& obj, AoiSlice* pPiece, AOI::AOI_Callback aoi_cb, void* cb_data)
	{
		ASSERT(pPiece->IsLocked());

		AoiSlice::ObjectSet& obj_list = pPiece->GetObjectList();
		AoiSlice::ObjectSet::const_iterator it = obj_list.begin();

		for (; it != obj_list.end(); ++it)
		{
			const AOI::object& tmpobj = *(*it);

			if (tmpobj.id == obj.id)
				continue;

			// watcher - marker
			if ( (obj.mode & MODE_WATCHER) &&
				 (tmpobj.mode & MODE_MARKER) )
			{
				AOI::ObjectInfo watcher;
				watcher.id    = obj.id;
				watcher.type  = obj.type;
				watcher.param = obj.param;

				AOI::ObjectInfo marker;
				marker.id    = tmpobj.id;
				marker.type  = tmpobj.type;
				marker.param = tmpobj.param;

				aoi_cb(aoitype, watcher, marker, cb_data);
			}

			// marker - watcher
			if ( (obj.mode & MODE_MARKER) &&
				 (tmpobj.mode & MODE_WATCHER) )
			{
				AOI::ObjectInfo watcher;
				watcher.id    = tmpobj.id;
				watcher.type  = tmpobj.type;
				watcher.param = tmpobj.param;

				AOI::ObjectInfo marker;
				marker.id    = obj.id;
				marker.type  = obj.type;
				marker.param = obj.param;

				aoi_cb(aoitype, watcher, marker, cb_data);
			}
		}
	}

	typedef void (*SliceEventCB)(AOI::AOI_Type aoitype, const AOI::object& obj, AoiSlice* pPiece, AOI::AOI_Callback aoi_cb, void* cb_data);

	/**
	 * @brief: scan slice
	 */
	class SendSliceMsg : public shared::copyable
	{
	public:
		SendSliceMsg(const AOI::object& obj, AOI::AOI_Type aoitype, SliceEventCB slice_cb, AOI::AOI_Callback aoi_cb, void* cb_data)
			: self_(obj),
			  aoi_type_(aoitype),
			  slice_func_(slice_cb),
			  aoi_cb_(aoi_cb),
			  cb_data_(cb_data)
		{ }

		inline void operator()(int index, AoiSlice* pPiece)
		{
			if (pPiece->IsEmpty())
				return;

			pPiece->Lock();
			slice_func_(aoi_type_, self_, pPiece, aoi_cb_, cb_data_);
			pPiece->Unlock();
		}
		
	private:
		AOI::object   self_;
		AOI::AOI_Type aoi_type_;
		SliceEventCB  slice_func_;
		AOI::AOI_Callback aoi_cb_;
		void*         cb_data_;
	};

	/**
	 * @brief: enter slice
	 */
	class RunnerEnter
	{
	public:
		RunnerEnter(const AOI::object& obj, SliceEventCB slice_cb, AOI::AOI_Callback aoi_cb, void* cb_data)
			: self_(obj),
			  slice_func_(slice_cb),
			  aoi_cb_(aoi_cb),
			  cb_data_(cb_data)
		{ }

		inline void operator()(AoiSlice* pPiece)
		{
			if (pPiece->IsEmpty())
				return;

			pPiece->Lock();
			slice_func_(AOI::OBJ_ENTER, self_, pPiece, aoi_cb_, cb_data_);
			pPiece->Unlock();
		}

	private:
		AOI::object   self_;
		SliceEventCB  slice_func_;
		AOI::AOI_Callback aoi_cb_;
		void*         cb_data_;
	};

	/**
	 * @brief: leave slice
	 */
	class RunnerLeave
	{
	public:
		RunnerLeave(const AOI::object& obj, SliceEventCB slice_cb, AOI::AOI_Callback aoi_cb, void* cb_data)
			: self_(obj),
			  slice_func_(slice_cb),
			  aoi_cb_(aoi_cb),
			  cb_data_(cb_data)
		{ }

		inline void operator()(AoiSlice* pPiece)
		{
			if (pPiece->IsEmpty())
				return;

			pPiece->Lock();
			slice_func_(AOI::OBJ_LEAVE, self_, pPiece, aoi_cb_, cb_data_);
			pPiece->Unlock();
		}

	private:
		AOI::object   self_;
		SliceEventCB  slice_func_;
		AOI::AOI_Callback aoi_cb_;
		void*         cb_data_;
	};
	
} // Anonymous


///
/// AOI
///
AOI::AOI()
	: plane_(NULL),
	  aoi_cb_(NULL),
	  cb_data_(NULL)
{
	plane_ = new AoiPlane();
	ASSERT(plane_);
}

AOI::~AOI()
{
	// delete grid
	DELETE_SET_NULL(plane_);

	// clear all object
	Clear();
}

bool AOI::Init(const rect& scope_rt, float step, float near, float far, AOI_Callback aoi_cb, void* cb_data)
{
	scope_rect_ = scope_rt;

	int row    = static_cast<int>((scope_rt.right - scope_rt.left)/step + COMPLEMENT_EPSINON);
	int column = static_cast<int>((scope_rt.bottom - scope_rt.top)/step + COMPLEMENT_EPSINON); 
	ASSERT(row >= 1 && column >= 1);

	AoiPlane::InitData initdata;
	initdata.row    = row;
	initdata.column = column;
	initdata.step   = step;
	initdata.startX = scope_rt.left;
	initdata.startY = scope_rt.top;
	initdata.near_vision = near;
	initdata.far_vision  = far;
	if (!plane_->Init(initdata))
	{
		LOG_ERROR << "aoi init error: plane Init() ERROR!";
		return false;
	}

	aoi_cb_  = aoi_cb;
	cb_data_ = cb_data;
	return true;
}

/**
 * @brief Clear
 * （1）清除aoi中所有对象，但是不释放plane，网格不变
 */
void AOI::Clear()
{
	// object
	{
		MutexLockTimedGuard lock(mutex_obj_map_);
		IDToObjectMap::iterator it = object_map_.begin();
		for (; it != object_map_.end(); ++it)
		{
			DeleteObject(it->second);
			it->second = NULL;
		}
		object_map_.clear();
	}

	// area object
	{
		MutexLockTimedGuard lock_area(mutex_area_map_);
		IDToAreaMap::iterator it_area = area_map_.begin();
		for (; it_area != area_map_.end(); ++it_area)
		{
			DeleteArea(it_area->second);
			it_area->second = NULL;
		}
		area_map_.clear();

		it_area = waiting_area_map_.begin();
		for (; it_area != waiting_area_map_.end(); ++it_area)
		{
			DeleteArea(it_area->second);
			it_area->second = NULL;
		}
		waiting_area_map_.clear();
	}

	area_mapto_objset_.clear();
	obj_mapto_areaset_.clear();

	aoi_cb_  = NULL;
	cb_data_ = NULL;
}

void AOI::AOI_Heartbeat()
{
	// load new area to area_map_
	AreaDoChange();
}

void AOI::AutoBroadcastCSMsg(const object& obj, AOI_Type aoitype)
{
	AoiSlice* pStart = plane_->Locate(obj.position.x, obj.position.y);
	ASSERT(pStart != NULL);
	pStart->Lock();
	slice_event(aoitype, obj, pStart, aoi_cb_, cb_data_);
	pStart->Unlock();

	plane_->ForEachSlice(pStart, SendSliceMsg(obj, aoitype, slice_event, aoi_cb_, cb_data_));
}

bool AOI::InsertDynamicObj(ObjectID id,
			               IDType type,
						   int64_t param,
			               const char* modestring,
			               const A2DVECTOR& init_pos,
			               bool area_detect)
{
	if (scope_rect_.IsOut(init_pos.x, init_pos.y))
	{
		LOG_ERROR << "object out of AOI scope!! obj_pos: " << init_pos.x << "," << init_pos.y;
		return false;
	}

	int mode = 0;
	for (int i = 0; modestring[i]; ++i)
	{
		char m = modestring[i];
		switch (m)
		{
			case 'w':
				mode |= MODE_WATCHER;
				break;
			case 'm':
				mode |= MODE_MARKER;
				break;
			default:
				assert(false);
				return false;
		}
	}

	object* obj = NewObject(id, mode, type, param, init_pos, area_detect);
	if (NULL == obj) return false;

	// area check, locked inside
	AreaDetection(obj, init_pos, false);

	// broadcast
	AutoBroadcastCSMsg(*obj, OBJ_ENTER);

	// insert to map
	{
		MutexLockTimedGuard lock(mutex_obj_map_);
		InsertObjToMap(obj); // this function will delete obj if insert false
	}
	return true;
}

bool AOI::DropDynamicObj(ObjectID id)
{
	object* obj = NULL;

	// remove from map
	{
		MutexLockTimedGuard lock(mutex_obj_map_);
		IDToObjectMap::iterator it_obj = object_map_.find(id);
		if (it_obj == object_map_.end())
		{
			LOG_ERROR << "Object:" << id << " not found in AOI system, operation is d";
			//assert(false);
			return false;
		}
		obj = it_obj->second;

		DropObjectFromMap(obj);
	}

	// area check, locked inside
	AreaDetection(obj, obj->position, true);

	// broadcast
	AutoBroadcastCSMsg(*obj, OBJ_LEAVE);

	// delete
	DeleteObject(obj);

	return true;
}

bool AOI::InsertPolygonArea(ObjectID id,
			                IDType type,
							const PolygonPointsVec& points)
{
	polygon_area* area = NewPolygonArea(id, type, points);
	if (NULL == area) return false;

	MutexLockTimedGuard lock(mutex_area_map_);
	if (!waiting_area_map_.insert(std::pair<ObjectID, polygon_area*>(area->id, area)).second)
	{
		DeleteArea(area);
		return false;
	}
	return true;
}

bool AOI::DropPolygonArea(ObjectID id)
{
	polygon_area* parea = NULL;

	MutexLockTimedGuard lock(mutex_area_map_);
	IDToAreaMap::iterator it_waiting = waiting_area_map_.find(id);
	if (it_waiting != waiting_area_map_.end())
	{
		DeleteArea(it_waiting->second);
		it_waiting->second = NULL;
		waiting_area_map_.erase(it_waiting);
		return true;
	}

	IDToAreaMap::iterator it = area_map_.find(id);
	if (it == area_map_.end())
	{
		assert(false);
		return false;
	}
	parea = it->second;
	area_map_.erase(it);

	AreaViewSaveMap::iterator it_area = area_mapto_objset_.find(parea->id);
	if (it_area != area_mapto_objset_.end())
	{
		ObjectIDSet& notify_set      = it_area->second;
		ObjectIDSet::iterator set_it = notify_set.begin();
		for (; set_it != notify_set.end(); ++set_it)
		{
			AreaViewSaveMap::iterator it_obj = obj_mapto_areaset_.find(*set_it);
			assert(it_obj != obj_mapto_areaset_.end());
			size_t n = it_obj->second.erase(parea->id);
			(void)n; assert(n == 1);
		}
		area_mapto_objset_.erase(it_area);
	}

	DeleteArea(parea);
	return true;
}

void AOI::AOI_Update(ObjectID id, const A2DVECTOR& new_pos)
{
	if (scope_rect_.IsOut(new_pos.x, new_pos.y))
	{
		LOG_ERROR << "object out of AOI scope! AOI_Update failure! obj_pos: " 
			<< new_pos.x << "," << new_pos.y;
		return;
	}

	object tmpobj;
	between_slice ret_slice;

	// lock mutex
	{
		object* obj = NULL;
		MutexLockTimedGuard lock(mutex_obj_map_);
		IDToObjectMap::iterator it_obj = object_map_.find(id);
		if (it_obj == object_map_.end())
		{
			LOG_ERROR << "Object:" << id << " not found in AOI system, operation is AOI_Update";
			//assert(false);
			return;
		}
		obj    = it_obj->second;
		tmpobj = *obj;

		// area check, locked inside
		AreaDetection(obj, new_pos, false);	

		// move
		ret_slice = MoveBetweenSlice(obj, new_pos);

		// update pos
		obj->position = new_pos;
	}

	if (ret_slice.is_cross_slice)
	{
		ASSERT(ret_slice.pPiece != NULL && ret_slice.pNewPiece != NULL);
		plane_->MoveBetweenSlice(ret_slice.pPiece, ret_slice.pNewPiece,
								 RunnerEnter(tmpobj, slice_event, aoi_cb_, cb_data_),
								 RunnerLeave(tmpobj, slice_event, aoi_cb_, cb_data_));
	}
}

AOI::between_slice AOI::MoveBetweenSlice(const object* obj, const A2DVECTOR& new_pos)
{
	AoiSlice* pPiece = plane_->Locate(obj->position.x, obj->position.y);
	AoiSlice* pNewPiece = plane_->Locate(new_pos.x, new_pos.y);
	ASSERT(pPiece != NULL && pNewPiece != NULL);

	between_slice ret_info;
	if (pNewPiece != pPiece)
	{
		ret_info.is_cross_slice = true;
		ret_info.pPiece         = pPiece;
		ret_info.pNewPiece      = pNewPiece;
		// double lock
		MutexLockTimedDoubleGuard lock(pPiece->obtain_mutex(), pNewPiece->obtain_mutex());
		pPiece->RemoveObject(obj);
		pNewPiece->InsertObject(obj);
	}

	return ret_info;
}

// needed lock outside
void AOI::InsertObjToMap(object* obj)
{
	assert(object_map_.insert(std::pair<ObjectID, object*>(obj->id, obj)).second);

	AoiSlice* pPiece = plane_->Locate(obj->position.x, obj->position.y);
	ASSERT(pPiece != NULL);
	pPiece->Lock();
	pPiece->InsertObject(obj);
	pPiece->Unlock();
}

// needed lock outside
void AOI::DropObjectFromMap(object* obj)
{
	size_t n = object_map_.erase(obj->id);
	ASSERT(n == 1);

	AoiSlice* pPiece = plane_->Locate(obj->position.x, obj->position.y);
	ASSERT(pPiece != NULL);
	pPiece->Lock();
	pPiece->RemoveObject(obj);
	pPiece->Unlock();
}

// needed lock outside
void AOI::InsertAreaToMap(polygon_area* area)
{
	assert(area_map_.insert(std::pair<ObjectID, polygon_area*>(area->id, area)).second);
}

AOI::object* AOI::NewObject(ObjectID id, int mode, IDType type, int64_t param, const A2DVECTOR& pos, bool area_detect)
{
	struct object* obj = new object();
	if (NULL == obj) return NULL;

	obj->id          = id;
	obj->type        = type;
	obj->mode        = mode;
	obj->position    = pos;
	obj->area_detect = area_detect;
	obj->param       = param;

	return obj;
}

AOI::polygon_area* AOI::NewPolygonArea(ObjectID id, IDType type, const PolygonPointsVec& points)
{
	polygon_area* parea = new polygon_area();
	if (NULL == parea) return NULL;

	parea->id        = id;
	parea->type      = type;
	parea->is_convex = false;
	for (size_t i = 0; i < points.size(); ++i)
	{
		parea->polygon.push_back(points[i]);
	}
	if (parea->polygon.is_convex_polygon())
	{
		parea->is_convex = true;
	}
	parea->rect = parea->polygon.aabb();

	return parea;
}

void AOI::AreaDoChange()
{
	if (!waiting_area_map_.empty())
	{
		IDToAreaMap tmp_areas;
		{
			MutexLockTimedGuard lock(mutex_area_map_);
			tmp_areas.swap(waiting_area_map_);
		}

		IDToAreaMap::iterator it = tmp_areas.begin();
		for (; it != tmp_areas.end(); ++it)
		{
			polygon_area* area = it->second;
			// refresh all objests in map
			{
				MutexLockTimedGuard objLock(mutex_obj_map_);
				IDToObjectMap::const_iterator it_obj = object_map_.begin();
				for (; it_obj != object_map_.end(); ++it_obj)
				{
					const object* obj = it_obj->second;
					if (!obj->area_detect)
						continue;
					if (CheckPointInArea(area, obj->position))
					{
						MutexLockTimedGuard lock(mutex_area_map_);
						EnterAreaCB(aoi_cb_, area, obj, cb_data_);
					}
					else
					{
						MutexLockTimedGuard lock(mutex_area_map_);
						LeaveAreaCB(aoi_cb_, area, obj, cb_data_);
					}
				}
			}

			MutexLockTimedGuard areaLock(mutex_area_map_);
			InsertAreaToMap(area);
		}
	}
}

void AOI::AreaDetection(const object* obj, const A2DVECTOR& pos, bool obj_is_drop)
{
	// not allow to detect area
	if (!obj->area_detect)
		return;

	// area detection
	if (!area_map_.empty())
	{
		MutexLockTimedGuard lock(mutex_area_map_);
		if (obj_is_drop)
		{
			ObjectDropFromArea(obj);
			return;
		}
		IDToAreaMap::const_iterator it_area = area_map_.begin();
		for (; it_area != area_map_.end(); ++it_area)
		{
			const polygon_area* parea = it_area->second;
			if (CheckPointInArea(parea, pos))
				EnterAreaCB(aoi_cb_, parea, obj, cb_data_);
			else
				LeaveAreaCB(aoi_cb_, parea, obj, cb_data_);
		}
	}
}

void AOI::ObjectDropFromArea(const object* obj)
{
	// it_otoa: obj to area
	AreaViewSaveMap::iterator it_otoa = obj_mapto_areaset_.find(obj->id);
	if (it_otoa != obj_mapto_areaset_.end())
	{
		ObjectIDSet notify_set       = it_otoa->second;
		ObjectIDSet::iterator set_it = notify_set.begin();
		for (; set_it != notify_set.end(); ++set_it)
		{
			polygon_area* area = NULL;
			IDToAreaMap::iterator it_area = area_map_.find(*set_it);
			if (it_area == area_map_.end()) continue;
			area = it_area->second;
			LeaveAreaCB(aoi_cb_, area, obj, cb_data_);

			// erase in area savemap
			AreaViewSaveMap::iterator tmp_it_atoo = area_mapto_objset_.find(area->id);
			assert(tmp_it_atoo != area_mapto_objset_.end());
			tmp_it_atoo->second.erase(obj->id);
		}
		it_otoa->second.clear();
		obj_mapto_areaset_.erase(obj->id);
	}
}

bool AOI::CheckPointInArea(const polygon_area* parea, const A2DVECTOR& pos) const
{
	if (parea->rect.point_in_rectangle(pos.x, pos.y))
	{
		if (parea->is_convex) 
		{
			if (parea->polygon.point_in_convex_polygon(pos.x, pos.y))
				return true;
			else
				return false;
		}
		else 
		{
			if (parea->polygon.point_in_polygon(pos.x, pos.y))
				return true;
			else
				return false;
		}
	}
	else
	{
		return false;
	}
}

void AOI::EnterAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data)
{
	ObjectIDSet::iterator it = area_mapto_objset_[area->id].find(obj->id);
	if (it == area_mapto_objset_[area->id].end())
	{
		ObjectInfo watcher_info, marker_info;
		watcher_info.id   = area->id;
		watcher_info.type = area->type;
		marker_info.id    = obj->id;
		marker_info.type  = obj->type;
		aoi_cb(OBJ_ENTER, watcher_info, marker_info, cb_data);

		area_mapto_objset_[area->id].insert(obj->id);
		obj_mapto_areaset_[obj->id].insert(area->id);
	}
}

void AOI::LeaveAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data)
{
	ObjectIDSet::iterator it = area_mapto_objset_[area->id].find(obj->id);
	if (it != area_mapto_objset_[area->id].end())
	{
		ObjectInfo watcher_info, marker_info;
		watcher_info.id   = area->id;
		watcher_info.type = area->type;
		marker_info.id    = obj->id;
		marker_info.type  = obj->type;
		aoi_cb(OBJ_LEAVE, watcher_info, marker_info, cb_data);

		area_mapto_objset_[area->id].erase(obj->id);
		obj_mapto_areaset_[obj->id].erase(area->id);
	}
}

} // namespace gamed
