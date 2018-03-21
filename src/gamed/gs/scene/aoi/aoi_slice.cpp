#include "aoi_slice.h"

#include "shared/base/mutex.h"


namespace gamed {

using namespace shared;

AoiSlice::AoiSlice()
{
}

AoiSlice::~AoiSlice()
{
	obj_set_.clear();
}

void AoiSlice::InsertObject(const AOI::object* pobj)
{
	ASSERT(IsLocked());
	bool ret = obj_set_.insert(pobj).second;;
	ASSERT(ret); (void)ret;
}

void AoiSlice::RemoveObject(const AOI::object* pobj)
{
	ASSERT(IsLocked());
	int ret = obj_set_.erase(pobj);
	ASSERT(ret == 1); (void)ret;
}

} // namespace gamed
