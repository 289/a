#include "aoi.h"

#include <assert.h>

#include "shared/logsys/logging.h"


namespace gamed {

namespace {

	enum Mode_Def
	{
		MODE_WATCHER = 0x0001,
		MODE_MARKER  = 0x0002,
		MODE_MOVE    = 0x0004,
		MODE_DROP    = 0x0008,
	};
	
} // Anonymous

AOI::AOI()
	: rad_short_(10.f),
	  rad_long_(20.f),
	  aoi_cb_(NULL),
	  cb_data_(NULL)
{
}

AOI::~AOI()
{
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
}

AOI::object* AOI::NewObject(ObjectID id, int mode, IDType type, int64_t param, const A2DVECTOR& last, const A2DVECTOR& pos, bool area_detect)
{
	struct object* obj = new object();
	if (NULL == obj) return NULL;

	obj->id          = id;
	obj->type        = type;
	obj->version     = 0;
	obj->mode        = mode;
	obj->last        = last;
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

bool AOI::ChangeMode(object* obj, bool set_watcher, bool set_marker)
{
	bool change = false;
	if (obj->mode == 0) 
	{
		if (set_watcher) 
		{
			obj->mode = MODE_WATCHER;
		}
		if (set_marker) 
		{
			obj->mode |= MODE_MARKER;
		}
		return true;
	}

	if (set_watcher) 
	{
		if (!(obj->mode & MODE_WATCHER)) 
		{
			obj->mode |= MODE_WATCHER;
			change = true;
		}
	} 
	else 
	{
		if (obj->mode & MODE_WATCHER) 
		{
			obj->mode &= ~MODE_WATCHER;
			change = true;
		}
	}

	if (set_marker) 
	{
		if (!(obj->mode & MODE_MARKER)) 
		{
			obj->mode |= MODE_MARKER;
			change = true;
		}
	} 
	else 
	{
		if (obj->mode & MODE_MARKER) 
		{
			obj->mode &= ~MODE_MARKER;
			change = true;
		}
	}

	return change;
}

// w(atcher) m(arker) d(rop)
void AOI::AOI_Update(ObjectID id, const char* modestring,  const A2DVECTOR& pos)
{
	int i;
	bool set_watcher = false;
	bool set_marker  = false;
	bool set_drop    = false;

	for (i=0; modestring[i]; ++i)
	{
		char m = modestring[i];
		switch (m)
		{
			case 'w':
				set_watcher = true;
				break;
			case 'm':
				set_marker  = true;
				break;
			case 'd':
				set_drop    = true;
				break;
			default:
				assert(false);
				return;
		}
	}
	
	// lock mutex
	object* obj = NULL;
	MutexLockTimedGuard lock(mutex_obj_map_);
	IDToObjectMap::iterator it_obj = object_map_.find(id);
	if (it_obj == object_map_.end())
	{
		LOG_ERROR << "Object:" << id << " not found in AOI system, operation is " << modestring;
		//assert(false);
		return;
	}
	obj = it_obj->second;

	// area check, locked inside
	AreaDetection(obj, pos, set_drop);	
	
	if (set_drop) 
	{
		obj->mode = MODE_DROP;
		return;
	}

	bool changed  = ChangeMode(obj, set_watcher, set_marker);

	obj->position = pos;
	if (changed || !is_near(pos, obj->last))
	{
		// new object or change object mode
		// or position changed
		obj->last  = pos;
		obj->mode |= MODE_MOVE;
		++obj->version;
	}
}

void AOI::DropPair(hot_pair* pair)
{
	pair->watcher = NULL;
	pair->marker  = NULL;
}

void AOI::FlushPair()
{
	PairList::iterator it = hot_list_.begin();
	for (; it != hot_list_.end();)
	{
		bool is_droped = false; 
		hot_pair* p = &(*it);
		if ( (p->watcher->version != p->watcher_version) ||
			 (p->marker->version != p->marker_version) ||
			 (p->watcher->mode & MODE_DROP) ||
			 (p->marker->mode & MODE_DROP) )
		{
			DropPair(p);
			is_droped = true;
		}
		else
		{
			float distance2 = dist2(p->watcher, p->marker);
			if (distance2 > rad_long_ * rad_long_)
			{
				LeaveViewCB(aoi_cb_, p->watcher, p->marker, cb_data_);
				DropPair(p);
				is_droped = true;
			}
			else if (distance2 < rad_short_ * rad_short_)
			{
				EnterViewCB(aoi_cb_, p->watcher, p->marker, cb_data_);
				DropPair(p);
				is_droped = true;
			}
		}

		if (is_droped)
			it = hot_list_.erase(it);
		else
			++it;
	}
}

void AOI::SetPush(object* obj)
{
	int mode = obj->mode;
	if (mode & MODE_WATCHER) 
	{
		if (mode & MODE_MOVE) 
		{
			assert(watcher_move_.insert(obj).second);
			obj->mode &= ~MODE_MOVE;
		}
		else
		{
			assert(watcher_static_.insert(obj).second);
		}
	}
	if (mode & MODE_MARKER) 
	{
		if (mode & MODE_MOVE) 
		{
			assert(marker_move_.insert(obj).second);
			obj->mode &= ~MODE_MOVE;
		}
		else
		{
			assert(marker_static_.insert(obj).second);
		}
	}
}

void AOI::GenHotPair(object* watcher, object* marker)
{
	if (watcher == marker)
		return;

	float distance2 = dist2(watcher, marker);
	if (distance2 < rad_short_ * rad_short_)
	{
		EnterViewCB(aoi_cb_, watcher, marker, cb_data_);
		return;
	}

	if (distance2 > rad_long_ * rad_long_)
	{
		LeaveViewCB(aoi_cb_, watcher, marker, cb_data_);
		return;
	}

	hot_pair p;
	p.watcher = watcher;
	p.marker  = marker;
	p.watcher_version = watcher->version;
	p.marker_version = marker->version;

	hot_list_.push_back(p);
}

void AOI::GenHotPairList(ObjectSet& watcher, ObjectSet& marker)
{
	ObjectSet::iterator it_watcher = watcher.begin();
	for (; it_watcher != watcher.end(); ++it_watcher)
	{
		ObjectSet::iterator it_marker  = marker.begin();
		for (; it_marker != marker.end(); ++it_marker)
		{
			GenHotPair(*it_watcher, *it_marker);
		}
	}
}

void AOI::AOI_Message(AOI_Callback aoi_cb, void* cb_data)
{
	aoi_cb_  = aoi_cb;
	cb_data_ = cb_data;

	// load new area to area_map_
	AreaDoChange();

	// flush hot pair
	FlushPair();

	ClearObjectSet(watcher_static_);
	ClearObjectSet(watcher_move_);
	ClearObjectSet(marker_static_);
	ClearObjectSet(marker_move_);

	{
		MutexLockTimedGuard lock(mutex_obj_map_);
		IDToObjectMap::iterator it_obj = object_map_.begin();
		for (; it_obj != object_map_.end();)
		{
			if (it_obj->second->mode & MODE_DROP)
			{
				DropObject(it_obj->second);
				object_map_.erase(it_obj++);
				continue;
			}
			else
			{
				SetPush(it_obj->second);
				++it_obj;
			}
		}
	}

	GenHotPairList(watcher_static_, marker_move_);
	GenHotPairList(watcher_move_, marker_static_);
	GenHotPairList(watcher_move_, marker_move_);
}

bool AOI::InsertAnnulusObj(ObjectID id, 
		                   IDType type,
						   int64_t param,
		                   const char* modestring, 
						   const A2DVECTOR& init_pos,
						   bool area_detect)
{
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

	// set to move_mode
	mode |= MODE_MOVE;	

	object* obj = NewObject(id, mode, type, param, init_pos, init_pos, area_detect);
	if (NULL == obj) return false;

	// area check, locked inside
	AreaDetection(obj, init_pos, false);	

	MutexLockTimedGuard lock(mutex_obj_map_);
	InsertObjToMap(obj); // this function will delete obj if insert false
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
					if (obj->mode & MODE_DROP) 
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

// needed lock outside
void AOI::DropObject(object* obj)
{
	ObjHotPairSaveMap::iterator m_it = marker_hotpair_savemap_.find(obj->id);
	if (m_it != marker_hotpair_savemap_.end())
	{
		ObjectIDSet notify_set       = m_it->second;
		ObjectIDSet::iterator set_it = notify_set.begin();
		for (; set_it != notify_set.end(); ++set_it)
		{
			object* watcher = NULL;
			IDToObjectMap::iterator it = object_map_.find(*set_it);
			if (it == object_map_.end()) continue;
			watcher = it->second;
			LeaveViewCB(aoi_cb_, watcher, obj, cb_data_);

			// erase in watcher savemap
			ObjHotPairSaveMap::iterator tmp_it = watcher_hotpair_savemap_.find(watcher->id);
			assert(tmp_it != watcher_hotpair_savemap_.end());
			tmp_it->second.erase(obj->id);
		}
		m_it->second.clear();
		marker_hotpair_savemap_.erase(obj->id);
	}

	ObjHotPairSaveMap::iterator w_it = watcher_hotpair_savemap_.find(obj->id);
	if (w_it != watcher_hotpair_savemap_.end())
	{
		w_it->second.clear();
		watcher_hotpair_savemap_.erase(obj->id);
	}

	DeleteObject(obj);
}

void AOI::PackObjectInfo(const object* obj, ObjectInfo& info)
{
	info.id    = obj->id;
	info.type  = obj->type;
	info.param = obj->param;
}

void AOI::PackAreaObjectInfo(const polygon_area* area, ObjectInfo& info)
{
	info.id    = area->id;
	info.type  = area->type;
	info.param = 0;
}

void AOI::EnterViewCB(AOI_Callback aoi_cb, const object* watcher, const object* marker, void* cb_data)
{
	ObjectIDSet::iterator it = watcher_hotpair_savemap_[watcher->id].find(marker->id);
	if (it == watcher_hotpair_savemap_[watcher->id].end())
	{
		ObjectInfo watcher_info, marker_info;
		PackObjectInfo(watcher, watcher_info);
		PackObjectInfo(marker, marker_info);
		aoi_cb(OBJ_ENTER, watcher_info, marker_info, cb_data);

		watcher_hotpair_savemap_[watcher->id].insert(marker->id);
		marker_hotpair_savemap_[marker->id].insert(watcher->id);
	}
}

void AOI::LeaveViewCB(AOI_Callback aoi_cb, const object* watcher, const object* marker, void* cb_data)
{
	ObjectIDSet::iterator it = watcher_hotpair_savemap_[watcher->id].find(marker->id);
	if (it != watcher_hotpair_savemap_[watcher->id].end())
	{
		ObjectInfo watcher_info, marker_info;
		PackObjectInfo(watcher, watcher_info);
		PackObjectInfo(marker, marker_info);
		aoi_cb(OBJ_LEAVE, watcher_info, marker_info, cb_data);

		watcher_hotpair_savemap_[watcher->id].erase(marker->id);
		marker_hotpair_savemap_[marker->id].erase(watcher->id);
	}
}

void AOI::EnterAreaCB(AOI_Callback aoi_cb, const polygon_area* area, const object* obj, void* cb_data)
{
	ObjectIDSet::iterator it = area_mapto_objset_[area->id].find(obj->id);
	if (it == area_mapto_objset_[area->id].end())
	{
		ObjectInfo watcher_info, marker_info;
		PackAreaObjectInfo(area, watcher_info);
		PackObjectInfo(obj, marker_info);
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
		PackAreaObjectInfo(area, watcher_info);
		PackObjectInfo(obj, marker_info);
		aoi_cb(OBJ_LEAVE, watcher_info, marker_info, cb_data);

		area_mapto_objset_[area->id].erase(obj->id);
		obj_mapto_areaset_[obj->id].erase(area->id);
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

// needed lock outside
void AOI::InsertObjToMap(object* obj)
{
	IDToObjectMap::iterator it_obj = object_map_.find(obj->id);
	if (it_obj != object_map_.end())
	{
		if (it_obj->second->mode & MODE_DROP)
		{
			DropObject(it_obj->second);
			object_map_.erase(it_obj);
		}
	}

	assert(object_map_.insert(std::pair<ObjectID, object*>(obj->id, obj)).second);
}

// needed lock outside
void AOI::InsertAreaToMap(polygon_area* area)
{
	if (!area_map_.insert(std::pair<ObjectID, polygon_area*>(area->id, area)).second)
	{
		assert(false);
	}
}

} // namespace gamed

