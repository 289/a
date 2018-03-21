#ifndef GAMED_GS_SCENE_AOI_SLICE_H_
#define GAMED_GS_SCENE_AOI_SLICE_H_

#include <math.h>
#include <set>

#include "shared/base/assertx.h"
#include "shared/base/mutex.h"
#include "shared/base/noncopyable.h"
#include "gs/global/math_types.h"

#include "aoi_if.h"


namespace gamed {

/**
 * @brief AoiSlice
 * （1）Slice使用前都需要加锁
 */
class AoiSlice : shared::noncopyable
{
public:
	AoiSlice();
	~AoiSlice();

	// **** thread unsafe ****
	void InsertObject(const AOI::object* pobj);
	void RemoveObject(const AOI::object* pobj);

	typedef std::set<const AOI::object*> ObjectSet;
	inline ObjectSet& GetObjectList();

	inline void  Lock();
	inline void  Unlock();
	inline bool  IsLocked();

	inline float Distance(AoiSlice* pslice) const;
	inline int   IsEmpty() const;

	inline bool  IsOutside(float x, float y) const;
	inline bool  IsInside(float x, float y) const;

	inline void  set_slice_range(const rect& range);
	inline rect  slice_range() const;

	inline shared::MutexLock& obtain_mutex();


private:
	shared::MutexLock obj_mutex_;
	ObjectSet         obj_set_;
	struct rect       slice_range_;
};

///
/// inline func
///
inline AoiSlice::ObjectSet& AoiSlice::GetObjectList()
{
	return obj_set_;
}

inline void AoiSlice::Lock()
{
	obj_mutex_.lock();
}

inline void AoiSlice::Unlock()
{
	obj_mutex_.unlock();
}

inline bool AoiSlice::IsLocked()
{
	return obj_mutex_.IsLockedByThisThread();
}

inline float AoiSlice::Distance(AoiSlice* pslice) const
{
	float dis_x = fabs(pslice->slice_range_.left - slice_range_.left);
	float dis_y = fabs(pslice->slice_range_.top - slice_range_.top);
	return dis_x > dis_y ? dis_x : dis_y;
}

inline int AoiSlice::IsEmpty() const
{
	return obj_set_.empty();
}

inline bool AoiSlice::IsOutside(float x, float y) const 
{ 
	return slice_range_.IsOut(x, y); 
}

inline bool AoiSlice::IsInside(float x, float y) const  
{ 
	return slice_range_.IsIn(x, y); 
}

inline void AoiSlice::set_slice_range(const rect& range)
{
	slice_range_ = range;
}

inline rect AoiSlice::slice_range() const 
{ 
	return slice_range_; 
}

inline shared::MutexLock& AoiSlice::obtain_mutex()
{
	return obj_mutex_;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_AOI_SLICE_H_
