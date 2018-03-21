#ifndef GAMED_GS_OBJ_AGGROLIST_H_
#define GAMED_GS_OBJ_AGGROLIST_H_

#include <vector>
#include <algorithm>

#include "gs/global/game_types.h"


namespace gamed {

/**
 * @brief AggroList 管理仇恨列表
 */
class AggroList
{
	struct AggroNode
	{
		AggroNode();
		AggroNode(const XID& a_id, int a_rage);
		bool operator==(const XID& rhs) const;

		XID id;
		int rage;
	};
	typedef std::vector<AggroNode> AggroListVec;

public:
	AggroList();
	~AggroList();

	int  AddRage(const XID& id, int rage);
	int  AddToFirst(const XID& id, int addon_rage);
	bool GetFirst(XID& target) const;

	int  Remove(const XID& id);
	void RemoveFirst();
	void Clear();

	inline bool   IsEmpty() const;
	inline size_t Size() const;
	inline bool   IsFirst(const XID& target) const;


private:
	inline AggroListVec::iterator Find(const XID& rhs);
	inline AggroListVec::iterator Find(int rage);

	static bool RageCompare(const AggroNode& lhs, const int rage);


private:
	AggroListVec alist_;
	size_t       max_size_;
};

///
/// inline func
///
inline bool AggroList::IsEmpty() const
{
	return alist_.empty();
}

inline size_t AggroList::Size() const
{
	return alist_.size();
}

inline bool AggroList::IsFirst(const XID& target) const
{
	return alist_.size() && alist_[0].id == target;
}

inline AggroList::AggroListVec::iterator AggroList::Find(const XID& rhs)
{
	return std::find(alist_.begin(), alist_.end(), rhs);
}

inline AggroList::AggroListVec::iterator AggroList::Find(int rage)
{
	return std::lower_bound(alist_.begin(), alist_.end(), rage, RageCompare);
}

} // namespace gamed

#endif // GAMED_GS_OBJ_AGGROLIST_H_
