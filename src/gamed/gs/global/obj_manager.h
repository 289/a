#ifndef GAMED_GS_GLOBAL_OBJ_MANAGER_H_
#define GAMED_GS_GLOBAL_OBJ_MANAGER_H_

#include <set>
#include <map>
#include <vector>
#include <deque>

#include "shared/base/noncopyable.h"
#include "shared/base/assertx.h"
#include "shared/base/base_define.h"
#include "shared/base/mutex.h"

#include "message.h"


namespace gamed {

using namespace shared;

///
/// ObjectAllocator
///
template <typename T>
class ObjectAllocator : shared::noncopyable
{
public:
	ObjectAllocator(size_t size)
		: pool_(NULL),
		  pool_size_(0),
		  allocated_count_(0)
	{
		// 现在采用固定pool大小的方式，因此初始大小就是最大值
		MutexLockGuard lock(mutex_);
		FirstExpand(size);
	}

	~ObjectAllocator()
	{
		DeleteAll();
	}

	T* Alloc()
	{
		MutexLockGuard lock(mutex_);
		if (obj_available_list_.empty())
			return NULL;

		T* tmp = obj_available_list_.front();
		obj_available_list_.pop_front();
		++allocated_count_;
		return tmp;
	}

	void Free(T* pObj)
	{
		assert(pObj);
		MutexLockGuard lock(mutex_);
		pObj->Release();
		obj_available_list_.push_back(pObj);
		--allocated_count_;
	}

	void DeleteAll()
	{
		MutexLockGuard lock(mutex_);
		obj_available_list_.clear();
		SAFE_DELETE_ARRAY(pool_);
		pool_size_ = 0;
	}

	inline T*     GetByIndex(size_t index) const;
	inline size_t GetIndex(T* obj) const;
	inline int    GetAllocatedCount() const;
	inline size_t GetCapacity() const;
	

private:
	// needed lock outside
	void FirstExpand(int size)
	{
		ASSERT(pool_ == NULL);
		pool_ = new T[size]; 
		ASSERT(pool_ != NULL);

		for (int i = 0; i < size; ++i)
		{
			obj_available_list_.push_back(pool_ + i);
		}
		pool_size_ = size;
	}


private:
	std::deque<T*> obj_available_list_;
	T*     pool_;
	size_t pool_size_;
	int    allocated_count_;

	MutexLock mutex_;
};

///
/// inline func
///
template <typename T>
inline T* ObjectAllocator<T>::GetByIndex(size_t index) const 
{
	ASSERT(index < pool_size_);
	return pool_ + index;
}

template <typename T>
inline size_t ObjectAllocator<T>::GetIndex(T* obj) const
{
	return obj - pool_;
}

template <typename T>
inline int ObjectAllocator<T>::GetAllocatedCount() const
{
	return allocated_count_;
}

template <typename T>
inline size_t ObjectAllocator<T>::GetCapacity() const
{
	return pool_size_;
}


///
/// ObjectManager
///
template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
class ObjectManager
{
	struct obj_info_record
	{
		obj_info_record() : ref_count(0) { }
		int32_t ref_count;
	};
	typedef std::set<T*> OBJECT_SET;
	typedef std::map<T*, obj_info_record> OBJECT_MAP;

public:
	typedef T* (*FindCallback)(int64_t);

	ObjectManager(FindCallback fcb)
		: allocator_(MAX_COUNT),
		  find_cb_(fcb)
	{ }

	~ObjectManager()
	{ }

	// ---- thread safe ----
	void    Insert(T* pObj);
	void    Remove(T* pObj);
	void    Heartbeat();

	// ---- thread safe ----
	T*      Alloc();
	void    Free(T* pObj);

	// ---- thread safe ----
	T*      Find(int64_t id) const;
	inline T*     GetByIndex(size_t index) const;
	inline size_t GetIndex(T* obj) const;

	// **** thread unsafe ****
	void    DeleteAllObjects();


protected:
	inline void DoDelete(T* pObj);
	inline void DoInsert(T* pObj);
	void    DoChange();
	void    CollectHeartbeatObject(std::vector<T*> &list);


private:
	ObjectAllocator<T> allocator_;

	mutable MutexLock  lock_change_;
	MutexLock          lock_heartbeat_;
	FindCallback       find_cb_;

	OBJECT_SET         obj_set_;
	OBJECT_MAP         change_list_;
	T*                 cur_obj_;
	int                heart_obj_count_;
	typename OBJECT_SET::iterator  cur_cursor_;
};


///
/// ---- template member function ----
///
template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
inline void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::DoDelete(T* pObj)
{
	if (cur_obj_ == pObj)
	{
		cur_cursor_ = obj_set_.find(cur_obj_);
		++cur_cursor_;
		if (obj_set_.end() == cur_cursor_)
		{
			cur_obj_ = NULL;
		}
		else 
		{
			cur_obj_ = *cur_cursor_;
		}
	}

	size_t n = obj_set_.erase(pObj);
	(void)n; ASSERT(n == 1);
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
inline void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::DoInsert(T* pObj)
{
	if (!obj_set_.insert(pObj).second)
	{
		// 多次插入报告错误
		ASSERT(false);
	}
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::DoChange()
{
	MutexLockGuard lock(lock_change_);
	typename OBJECT_MAP::const_iterator it = change_list_.begin();
	for (; it != change_list_.end(); ++it)
	{
		if (!it->second.ref_count) continue;
		ASSERT(it->second.ref_count == -1 || it->second.ref_count == 1 || it->second.ref_count == 0);
		if (it->second.ref_count > 0)
		{
			DoInsert(it->first);
		}
        else if (it->second.ref_count == 0) 
		{
			// 极端情况，池很紧张，刚free的obj又分配出去，而且没有来得急做DoChange
			continue;
		}
		else
		{
			DoDelete(it->first);
		}
	}
	change_list_.clear();
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::DeleteAllObjects()
{
	OBJECT_SET deleted_set;

	{
		MutexLockGuard lock(lock_change_);
		typename OBJECT_MAP::iterator it_change = change_list_.begin();
		for (; it_change != change_list_.end(); ++it_change)
		{
			if (it_change->second.ref_count > 0)
			{
				Free(it_change->first);
			}
			else
			{
				deleted_set.insert(it_change->first);
			}
		}
		change_list_.clear();
	}

	{
		MutexLockGuard lock(lock_heartbeat_);
		typename OBJECT_SET::iterator it_obj = obj_set_.begin();
		for (; it_obj != obj_set_.end(); ++it_obj)
		{
			typename OBJECT_SET::const_iterator it_del = deleted_set.find(*it_obj);
			if (it_del == deleted_set.end())
			{
				Free(*it_obj);
			}
		}
		obj_set_.clear();
	}

	allocator_.DeleteAll();
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::CollectHeartbeatObject(std::vector<T*> &list)
{
	DoChange();
	int size          = obj_set_.size();
	heart_obj_count_ += size;
	if (size == 0)
	{
		heart_obj_count_ = 0;
		cur_obj_         = NULL;
		return ;
	}

	typename std::set<T*>::iterator end = obj_set_.end();
	if (NULL == cur_obj_)
	{
		cur_cursor_ = end;
	}
	else
	{
		cur_cursor_ = obj_set_.find(cur_obj_);
	}

#ifdef _DEBUG
	int idle_count = 0;
#endif // _DEBUG

	list.reserve(size/MM_HEARTBEAT_TICK + 5);
	for (; heart_obj_count_ >= (int)MM_HEARTBEAT_TICK; heart_obj_count_ -= (int)MM_HEARTBEAT_TICK)
	{
		if (cur_cursor_ == end)
		{
			cur_cursor_ = obj_set_.begin();
		}

		cur_obj_ = *cur_cursor_;

		// 将这个对象加入到收集列表中
#ifdef _DEBUG
		idle_count += Insertor::push_back(list, cur_obj_);
#else // !_DEBUG
		Insertor::push_back(list, cur_obj_);
#endif // _DEBUG
		++cur_cursor_;
	}

	if (cur_cursor_ != end)
	{
		cur_obj_ = *cur_cursor_;
	}
	else 
	{
		cur_obj_ = NULL;
	}
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
T* ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Alloc()
{
	return allocator_.Alloc();
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Free(T* pObj)
{
	allocator_.Free(pObj);
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Insert(T* pObj)
{
	MutexLockGuard lock(lock_change_);
	change_list_[pObj].ref_count++;
}
	
template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Remove(T* pObj)
{
	MutexLockGuard lock(lock_change_);
	change_list_[pObj].ref_count--;
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
T* ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Find(int64_t id) const
{
	return find_cb_(id);
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
inline T* ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::GetByIndex(size_t index) const
{
	return allocator_.GetByIndex(index);
}
	
template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
inline size_t ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::GetIndex(T* obj) const
{
	return allocator_.GetIndex(obj);
}

template <typename T, size_t MM_HEARTBEAT_TICK, typename Insertor, size_t MAX_COUNT>
void ObjectManager<T, MM_HEARTBEAT_TICK, Insertor, MAX_COUNT>::Heartbeat()
{
	std::vector<T*> list;
	{
		MutexLockGuard lock(lock_heartbeat_);
		CollectHeartbeatObject(list);
	}
	if (!list.size()) return;

	MSG msg;
	memset(&msg, 0, sizeof(msg));
	msg.message = GS_MSG_OBJ_HEARTBEAT;
	msg.param   = MM_HEARTBEAT_TICK/TICK_PER_SEC;

	typename std::vector<T*>::iterator it  = list.begin();
	typename std::vector<T*>::iterator end = list.end(); 
	for (; it != end; ++it)
	{
		T* obj = *it;
		obj->Lock();
		if (!obj->IsActived()) 
		{
			obj->Unlock();
			continue;
		}
		int rst = -1;
		ASSERT(obj->world_plane());
		rst = obj->DispatchMessage(msg);
		obj->Unlock();
#ifdef __DEBUG_HEARTBEAT__
		if (rst)
		{
			__PRINTF("DispatchMessage() GS_MSG_OBJ_HEARTBEAT error!");
		}
#endif // __DEBUG_HEARTBEAT__
	}
}

} // namespace gamed

#endif // GAMED_GS_GLOBAL_OBJ_MANAGER_H_
