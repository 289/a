#ifndef COMMON_OBJDATAPOOL_OBJ_DATA_H_
#define COMMON_OBJDATAPOOL_OBJ_DATA_H_

#include <deque>
#include <map>

#include "shared/base/assertx.h"
#include "shared/base/base_define.h"
#include "shared/base/mutex.h"
#include "shared/base/rwlock.h"
#include "shared/base/noncopyable.h"


namespace common {

using namespace shared;

class ObjectData : public noncopyable
{
public:
	ObjectData()
		: is_attached_(false)
	{ }

	virtual ~ObjectData() { }
	virtual void Release() = 0;

	inline void set_is_attached(bool attached) { is_attached_ = attached; }
	inline bool is_attached() const { return is_attached_; }

	RWLock& get_rwlock() { return rwlock_; }

	void rdlock() 
	{
		rwlock_.timed_rdlock();
	}

	void wrlock() 
	{
		rwlock_.timed_wrlock();
	}

	void unlock() 
	{
		rwlock_.unlock();
	}

private:
	bool      is_attached_;
	RWLock    rwlock_;
};

template <typename T, size_t kMaxCount>
class ObjectDataAllocator : noncopyable
{
	static const int kExpansionInterval = 1024;
public:
	ObjectDataAllocator(size_t init_size)
		: allocated_count_(0)
	{
		assert(init_size <= kMaxCount);
		MutexLockGuard lock(mutex_);
		allocated_count_ += Expansion(init_size);
	}

	~ObjectDataAllocator()
	{
		MutexLockGuard lock(mutex_);
		for (size_t i = 0; i < obj_available_list_.size(); ++i)
		{
			SAFE_DELETE(obj_available_list_[i]);
		}
		obj_available_list_.clear();
	}

	T* Alloc()
	{
		MutexLockGuard lock(mutex_);
		if (obj_available_list_.empty())
		{
			allocated_count_ += Expansion(kExpansionInterval);
			if (allocated_count_ > kMaxCount || obj_available_list_.empty()) {
				assert(false);
			}
		}
		T* tmp = obj_available_list_.front();
		obj_available_list_.pop_front();
		return tmp;
	}

	void Free(T* pObj)
	{
		assert(pObj);
		MutexLockGuard lock(mutex_);
		pObj->Release();
		obj_available_list_.push_back(pObj);
	}

	int get_allocated_count() const 
	{ 
		return allocated_count_; 
	}

private:
	int Expansion(int size)
	{
		int tmpcount = 0;
		for (int i = 0; i < size; ++i)
		{
			T* tmp = new T();
			if (NULL != tmp)
			{
				obj_available_list_.push_back(tmp);
				++tmpcount;
			}
		}
		return tmpcount;
	}

private:
	std::deque<T*> obj_available_list_;
	size_t     allocated_count_;

	MutexLock  mutex_;
};


template <typename K, typename T, size_t kMaxCount>
class ObjectDataManager : noncopyable
{
	static const int kExecuteSuccess     = 0;
	static const int kErrObjectNotFound  = -1;

public:
	ObjectDataManager(int init_size) 
		: allocator_(init_size)
	{ }

	///
	/// only player-agent/other-agent themselves can call this func
	/// Attach a Read-Write ObjectData Pointer
	///
	T* Attach(K key)
	{
		MutexLockGuard lock(mutex_);
		T* pObj = NULL;
		typename ObjectDataMap::iterator it = objectdata_map_.find(key);
		if (objectdata_map_.end() == it)
		{
			pObj = AllocObjectData(key);
		}
		else
		{
			pObj = it->second;
		}

		ObjectData* ptmp = static_cast<ObjectData*>(pObj);
		assert(!ptmp->is_attached());
		ptmp->set_is_attached(true);
		return pObj;
	}

	// only player_agent/other-agent themselves can call this func
	// Detach a Read-Write ObjectData Pointer
	int Detach(K key)
	{
		MutexLockGuard lock(mutex_);
		typename ObjectDataMap::iterator it = objectdata_map_.find(key);
		if (objectdata_map_.end() == it)
			return kErrObjectNotFound;

		// give back to allocator
		allocator_.Free(it->second);

		ObjectData* ptmp = static_cast<ObjectData*>(it->second);
		assert(ptmp->is_attached());
		ptmp->set_is_attached(false);

		objectdata_map_.erase(it); 
		it = objectdata_map_.end();
		return kExecuteSuccess;
	}

	///
	/// Must be paired to call with DetachReadOnly()
	///
	const T* AttachReadOnly(K key)
	{
		MutexLockGuard lock(mutex_);
		typename ObjectDataMap::iterator it = objectdata_map_.find(key);
		if (objectdata_map_.end() == it)
			return NULL;

		ObjectData* ptmp = static_cast<ObjectData*>(it->second);
		ptmp->rdlock();
		return it->second;
	}

	// Must be paired to call with AttachReadOnly()
	int DetachReadOnly(K key)
	{
		MutexLockGuard lock(mutex_);
		typename ObjectDataMap::iterator it = objectdata_map_.find(key);
		if (objectdata_map_.end() == it)
			return kErrObjectNotFound;

		ObjectData* ptmp = static_cast<ObjectData*>(it->second);
		ptmp->unlock();
		return kExecuteSuccess;
	}

	// Must be paired to call with AttachReadOnly()
	int DetachReadOnly(T* pobj)
	{
		pobj->unlock();	
		return kExecuteSuccess;
	}


protected:
	// needed lock outside
	T* AllocObjectData(K key)
	{
		typename ObjectDataMap::iterator it = objectdata_map_.find(key);
		if (objectdata_map_.end() != it)
		{
			it->second->Release();
			return it->second;
		}

		T* ptmp = allocator_.Alloc();
		if (NULL == ptmp || !objectdata_map_.insert(std::make_pair(key, ptmp)).second)
		{
			assert(false);
			return NULL;
		}

		return ptmp;
	}


private:
	shared::MutexLock mutex_;
	ObjectDataAllocator<T, kMaxCount> allocator_;

	typedef std::map<K,T*> ObjectDataMap;
	ObjectDataMap  objectdata_map_;
};

} // namespace common

#endif // COMMON_OBJDATAPOOL_OBJ_DATA_H_
