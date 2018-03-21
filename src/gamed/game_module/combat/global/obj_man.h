#ifndef __GAME_MODULE_COMBAT_OBJ_MANAGER_H__
#define __GAME_MODULE_COMBAT_OBJ_MANAGER_H__

#include <set>
#include <map>
#include <vector>
#include <assert.h>
#include <algorithm>

#include "shared/base/mutex.h"

namespace combat
{

/**
 * @class ObjectPool
 * @breif 提供给战斗系统专用的对象池
 * @brief 对象池容量固定，不支持动态扩展
 */
template <typename T>
struct ObjectPool
{
private:
	T * pool_;
	T * header_;
	T * tail_;
	size_t count_;
	size_t pool_size_;
	shared::MutexLock lock_;

public:
	ObjectPool()
		: pool_(NULL),
		  header_(NULL),
		  tail_(NULL),
		  count_(0),
		  pool_size_(0)
	{
	}
	~ObjectPool()
	{
		pool_      = NULL;
		header_    = NULL;
		tail_      = NULL;
		count_     = 0;
		pool_size_ = 0;
	}

	bool Init(size_t size)
	{
		pool_ = new T[size];
		
		for (size_t i = 0; i < size; ++ i)
		{
			pool_[i].SetID(i);
			pool_[i].pPrev   = pool_ + (i-1);
			pool_[i].pNext   = pool_ + (i+1);
		}

		pool_[0].pPrev = NULL;
		pool_[size- 1].pNext = NULL;
		header_ = pool_;
		tail_ = pool_ + (size- 1);
		pool_size_ = size;
		return true;
	}

	void Release()
	{
		delete [] pool_;

		pool_      = NULL;
		header_    = NULL;
		tail_      = NULL;
		count_     = 0;
		pool_size_ = 0;
	}

	T* GetPool() const;
	T* GetByIndex(size_t index) const;
	size_t GetIndex(const T* obj) const;
	size_t ObjCount() const;
	size_t TotalCount() const;
	size_t ElementSize() const;

	T * Alloc()
	{
		shared::MutexLockGuard keeper(lock_);
		if (!header_) return NULL;
		T* pObj = header_;
		header_ = (T*)header_->pNext;
		if (header_)
		{
			header_->pPrev = NULL;
		}
		else
		{
			tail_ = NULL;
		}

		count_ ++;
		assert(!pObj->IsActived());
		
		pObj->Lock();
		assert(!pObj->IsActived());
		pObj->SetActive();

		//对象已加锁
		return pObj;
	}
	
	bool Free(T * pObj)	
	{
		//要求传入的结构保持锁定

		if (pObj->IsActived())
		{
			pObj->Clear();
		}

		shared::MutexLockGuard keeper(lock_);
		if (!tail_)
		{
			header_ = tail_ = pObj;
			pObj->pPrev = pObj->pNext = NULL;
		}
		else
		{
			tail_->pNext = pObj;
			pObj->pPrev = tail_;
			pObj->pNext = NULL;
			tail_ = pObj;
		}

		count_ --;
		return true;
	}
};


///
/// inline func
///
template <typename T>
inline T* ObjectPool<T>::GetPool() const
{
	return pool_;
}

template <typename T>
inline T* ObjectPool<T>::GetByIndex(size_t index) const
{
	assert(index < pool_size_);
	return pool_ + index;
} 

template <typename T>
inline size_t ObjectPool<T>::GetIndex(const T* obj) const
{
	return obj - pool_;
}

template <typename T>
inline size_t ObjectPool<T>::ObjCount() const 
{
	return count_;
}

template <typename T>
inline size_t ObjectPool<T>::TotalCount() const
{
	return pool_size_;
}

template <typename T>
inline size_t ObjectPool<T>::ElementSize() const
{
	return sizeof(T);
}

template <typename OBJECT, size_t HB_TICK_PER_SEC, typename Insertor>
class ObjectMan : public ObjectPool<OBJECT>
{
private:
	typedef OBJECT T;
	typedef std::set<OBJECT*>      OBJECT_SET;
	typedef std::map<OBJECT*, int> OBJECT_MAP;
	typedef std::vector<OBJECT*>   OBJECT_LIST;

	OBJECT_SET  obj_set_;
	OBJECT_MAP  change_map_;
	OBJECT*     cur_obj_;
	size_t      hb_obj_count_;

	shared::MutexLock lock_change_;
	shared::MutexLock lock_heartbeat_;

	void DoDelete(T * pObj)
	{
		if (pObj == cur_obj_)
		{
			typename std::set<OBJECT*>::iterator it = obj_set_.find(cur_obj_);
			++ it;
			if (it != obj_set_.end())
			{
				cur_obj_ = *it;
			}
			else
			{
				cur_obj_ = NULL;
			}
		}

		if (!obj_set_.erase(pObj))
		{
			ASSERT(false && "二次删除");
		}
	}

	void DoInsert(T * pObj)
	{
		if (!obj_set_.insert(pObj).second)
		{
			ASSERT(false && "二次插入");
		}
	}

	void DoChange()
	{
		shared::MutexLockGuard keeper(lock_change_);
		typename std::map<OBJECT *,int>::const_iterator it = change_map_.begin();
		for ( ;it != change_map_.end(); ++it)
		{
			if (!it->second) continue;
			ASSERT(it->second == -1 || it->second == 1 || it->second == 0);
			if (it->second > 0)
			{
				DoInsert(it->first);
			}
			else if (it->second == 0)
			{
				//对象池资源紧张,刚释放的对象还没从obj_set清除，立即又被重新分配出去.
				continue;
			}
			else
			{
				DoDelete(it->first);
			}
		}

		change_map_.clear();
	}

	void CollectHeartBeatObject(OBJECT_LIST& list)
	{
		DoChange();

		list.clear();
		typename std::set<OBJECT*>::iterator it = obj_set_.begin();
		for (; it != obj_set_.end(); ++ it)
		{
			Insertor::push_back(list, *it);
		}
	}

public:
	ObjectMan(): cur_obj_(NULL), hb_obj_count_(0) {}
	~ObjectMan() {}
	
	bool Init(size_t size)
	{
		ObjectPool<OBJECT>::Init(size);
		return true;
	}
	
	void Insert(T* pObj)
	{
		shared::MutexLockGuard keeper(lock_change_);
		change_map_[pObj] ++;
	}

	void Remove(T* pObj)
	{
		shared::MutexLockGuard keeper(lock_change_);
		change_map_[pObj] --;
	}
    
	void HeartBeat()
	{
		OBJECT_LIST list;
        {
            shared::MutexLockGuard keeper(lock_heartbeat_);
            CollectHeartBeatObject(list);
        }
        if (!list.size()) return;

		typename OBJECT_LIST::iterator it = list.begin();
		for (; it != list.end(); ++ it)
		{
			T* obj = *it;
			assert(obj);

			obj->Lock();
			if (!obj->IsActived())
			{
				obj->Unlock();
				continue;
			}

			obj->HeartBeat();
			obj->Unlock();
		}
	}
};

};

#endif
