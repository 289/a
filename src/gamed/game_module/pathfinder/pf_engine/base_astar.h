#ifndef __PF_ASTAR_BASE_H__
#define __PF_ASTAR_BASE_H__

#include <sys/types.h>
#include <stdint.h>
#include <vector>
#include <assert.h>

#include "game_stl.h"


//#ifdef PLATFORM_WINDOWS
//#include <hash_map>
//#elif PLATFORM_LINUX
//#include <backward/hash_map>
//#if __WORDSIZE == 64
//#else
//namespace __gnu_cxx
//{
//	template<>
//		struct hash<int64_t>
//		{
//			size_t operator()(int64_t __x) const
//			{ return __x; }
//		};
//};
//#endif
//#elif PLATFORM_ANDROID
//#include <backward/hash_map>
//namespace __gnu_cxx
//{
//	template<>
//		struct hash<int64_t>
//		{
//			size_t operator()(int64_t __x) const
//			{ return __x; }
//		};
//};
//#else
//#endif

namespace pathfinder
{

typedef int32_t Key32;
typedef int64_t Key64;
typedef int32_t Index;

#define GEN_KEY(x,z)    ((Key32)(((x) & 0xffff) | (((z) & 0xffff) << 16)))
#define MAKE_KEY(o,x,z) ((Key64)(((x) & 0xffff) | (((z) & 0xffff) << 16) | ((((Key64)(o) & 0xffff)) << 32)))
#define GET_KEY_X(key)  ((int16_t)((key)  & 0xffff))
#define GET_KEY_Z(key)  ((int16_t)(((key) & 0xffff0000) >> 16))

/**
 * @class Openlist
 * @brief list of visited nodes, keep heap-attribute while operate
 */
template<class T>
class Openlist
{
public:
	Openlist(){}
	virtual ~Openlist(){}

	virtual void Push(T* obj) = 0;
	virtual void Pop() = 0;
	virtual void Update(T* obj) = 0;
	virtual T* Top() const = 0;
	virtual T* Find(const T& obj) = 0;
	virtual bool Empty() const = 0;
};

template<class T>
struct NodeFinder
{
	T obj;
	NodeFinder(const T& t): obj(t) {}
	bool operator() (const T* t) const
	{
		return obj == *t;
	}
};

template<class T>
class OpenArray : public Openlist<T>
{
protected:
	struct CmpCost
	{
		bool operator() (const T* t1, const T* t2) const
		{
			return t1->cost > t2->cost;
		}
	};

private:
	typedef std::vector<T*> LIST;
	LIST list;

public:
	OpenArray() {}
	virtual ~OpenArray()
	{
		assert(list.empty());
	}

	virtual void Push(T* obj)
	{
		if (!obj) return;
		list.push_back(obj);
		std::push_heap(list.begin(), list.end(), CmpCost());
	}

	virtual void Pop()
	{
		if (list.empty()) return;
		std::pop_heap(list.begin(), list.end(), CmpCost());
		list.pop_back();
	}

	virtual T* Find(const T& obj)
	{
		typename LIST::const_iterator it = std::find_if(list.begin(), list.end(), NodeFinder<T>(obj));
		return it != list.end() ? *it : NULL;
	}

	virtual void Update(T* obj)
	{
		if (Empty()) return;
		std::make_heap(list.begin(), list.end(), CmpCost());
	}

	virtual T* Top() const
	{
		return Empty() ? NULL : list.front();
	}

	virtual bool Empty() const
	{
		return list.empty();
	}

	virtual void Clear()
	{
		for (size_t i = 0; i < list.size(); ++ i)
		{
			delete list[i];
			list[i] = NULL;
		}
		list.clear();
	}
};


template<class T>
class OpenHeap : public Openlist<T>
{
private:
	typedef std::vector<T*> LIST;

//#ifdef PLATFORM_WINDOWS
//	typedef stdext::hash_map<Key64, Index> Pos2Index;
//#else
//	typedef __gnu_cxx::hash_map<Key64, Index> Pos2Index;
//#endif
  typedef stl_hash_map<Key64, Index> Pos2Index;

	LIST list;
	Pos2Index pos2idx;

public:
	OpenHeap()
	{
	}
	virtual ~OpenHeap()
	{
		assert(list.empty());
		assert(pos2idx.empty());
	}

	virtual void Push(T* obj)
	{
		if (!obj) return;

		Key64 key = MAKE_KEY(obj->GetOwner(), obj->GetCoord().x, obj->GetCoord().z);
		pos2idx[key] = list.size();
		list.push_back(obj);
		HeapifyUp(list.size() - 1);
	}

	virtual void Pop()
	{
		if (list.empty()) return;

		Swap(0, list.size() - 1);

		Key64 key = MAKE_KEY(list.back()->GetOwner(), list.back()->GetCoord().x, list.back()->GetCoord().z);
		pos2idx.erase(key);
		list.pop_back();

		HeapifyDown(0);
	}

	virtual void Update(T* obj)
	{
		Key64 key = MAKE_KEY(obj->GetOwner(), obj->GetCoord().x, obj->GetCoord().z);
		HeapifyUp(pos2idx[key]);
	}

	virtual T* Top() const
	{
		return Empty() ? NULL : list.front();
	}

	virtual T* Find(const T& obj)
	{
		Key64 key = MAKE_KEY(obj.GetOwner(), obj.GetCoord().x, obj.GetCoord().z);
    Pos2Index::iterator it = pos2idx.find(key);
		if (it == pos2idx.end())
		{
			return NULL;
		}
		return list[it->second];
	}

	virtual bool Empty() const
	{
		return list.empty() && pos2idx.empty();
	}

	void Clear()
	{
		for (size_t i = 0; i < list.size(); ++ i)
			delete list[i];

		list.clear();
		pos2idx.clear();
	}

private:
	void HeapifyUp(int index)
	{
		int parent = 0;
		while (index > 0)
		{
			// got parent node-idx and compare
			parent = (index-1) / 2;
			if (list[parent]->GetCost() > list[index]->GetCost())
			{
				Swap(index, parent);
				index = parent;
			}
			else
			{
				return;
			}
		};
	}

	void HeapifyDown(int index)
	{
		int min_child   = 0;
		int left_child  = 0;
		int right_child = 0;

		while (index < (int)(list.size()))
		{
			left_child  = 2*index + 1;
			right_child = 2*index + 2;

			// find minimum child
			if (left_child >= (int)(list.size()))
			{
				return;
			}
			else if (right_child >= (int)(list.size()))
			{
				min_child = left_child;
			}
			else if (list[left_child]->GetCost() < list[right_child]->GetCost())
			{
				min_child = left_child;
			}
			else
			{
				min_child = right_child;
			}

			// compare with minimum-child
			if (list[min_child]->GetCost() < list[index]->GetCost())
			{
				Swap(min_child, index);
				index = min_child;
			}
			else
			{
				return;
			}
		}
	}

	void Swap(int idx1, int idx2)
	{
		T* obj1 = list[idx1];
		T* obj2 = list[idx2];

		Key64 key1 = MAKE_KEY(obj1->GetOwner(), obj1->GetCoord().x, obj1->GetCoord().z);
		Key64 key2 = MAKE_KEY(obj2->GetOwner(), obj2->GetCoord().x, obj2->GetCoord().z);

		list[idx2] = obj1;
		list[idx1] = obj2;

		pos2idx[key1] = idx2;
		pos2idx[key2] = idx1;
	}
};


/**
 * @class CloseList
 * @brief list of node which has been visited and selected from openlist
 */
template<class T>
class Closelist
{
public:
	Closelist() {}
	virtual ~Closelist() {}

	virtual void Push(T* obj) = 0;
	virtual void Remove(const T* obj) = 0;
	virtual T* Find(const T& obj) = 0;
	virtual void Clear() = 0;
	virtual void Dump() {}
};

template<class T>
class CloseArray : public Closelist<T>
{
	typedef std::vector<T*> LIST;
	LIST list;

public:
	CloseArray()
	{
	}
	virtual ~CloseArray()
	{
		assert(list.empty());
	}

	virtual void Push(T* obj)
	{
		if (!obj) return;
		list.push_back(obj);
	}

	virtual void Remove(const T* obj)
	{
		if (!obj) return;
		typename LIST::iterator it = std::find_if(list.begin(), list.end(), NodeFinder<T>(*obj));
		if (it == list.end())
		{
			assert(false);
			return;
		}
		list.erase(it);
	}

	virtual T* Find(const T& obj)
	{
		typename LIST::const_iterator it = std::find_if(list.begin(), list.end(), NodeFinder<T>(obj));
		return it != list.end() ? *it : NULL;
	}

	virtual void Clear()
	{
		for (size_t i = 0; i < list.size(); ++ i)
		{
			delete list[i];
			list[i] = NULL;
		}
		list.clear();
	}
};

template<class T>
class CloseHash : public Closelist<T>
{
private:
//#ifdef PLATFORM_WINDOWS
//	typedef stdext::hash_map<Key64, T*> HashMap;
//#else
//	typedef __gnu_cxx::hash_map<Key64, T*> HashMap;
//#endif
  typedef stl_hash_map<Key64, T*> HashMap;

	HashMap _map;

public:
	CloseHash()
	{
	}
	virtual ~CloseHash()
	{
		assert(_map.empty());
	}

	virtual void Push(T* obj)
	{
		if (!obj) return;
		Key64 key = MAKE_KEY(obj->GetOwner(), obj->GetCoord().x, obj->GetCoord().z);
		_map[key] = obj;
	}

	virtual void Remove(const T* obj)
	{
		if (!obj) return;
		Key64 key = MAKE_KEY(obj->GetOwner(), obj->GetCoord().x, obj->GetCoord().z);
		_map.erase(key);
	}

	virtual T* Find(const T& obj)
	{
		Key64 key = MAKE_KEY(obj.GetOwner(), obj.GetCoord().x, obj.GetCoord().z);
		typename HashMap::iterator it = _map.find(key);
		return it != _map.end() ? it->second : NULL;
	}

	virtual void Clear()
	{
		typename HashMap::iterator it = _map.begin();
		for (; it != _map.end(); ++ it)
			delete it->second;

		_map.clear();
	}

	virtual void Dump()
	{
		typename HashMap::iterator it = _map.begin();
		for (; it != _map.end(); ++ it)
			fprintf(stdout, "(%d,%d),", it->second->GetCoord().x, it->second->GetCoord().z);
	}
};

};

#endif // __PF_ASTAR_BASE_H__
