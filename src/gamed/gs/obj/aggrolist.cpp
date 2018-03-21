#include "aggrolist.h"

#include "gs/global/game_def.h"


namespace gamed {

///
/// AggroList::AggroNode
///
AggroList::AggroNode::AggroNode()
	: rage(0)
{
}

AggroList::AggroNode::AggroNode(const XID& a_id, int a_rage)
	: id(a_id),
	  rage(a_rage)
{
}

bool AggroList::AggroNode::operator==(const XID& rhs) const
{
	return id == rhs;
}


///
/// AggroList
///
bool AggroList::RageCompare(const AggroNode& lhs, const int rage)
{
	return lhs.rage >= rage;
}

AggroList::AggroList()
	: max_size_(kMaxAggroListSize)
{
}

AggroList::~AggroList()
{
}

int AggroList::AddRage(const XID& id, int a_rage)
{
	if (a_rage == 0) 
		return -1;

	AggroListVec::iterator it  = Find(id);
	AggroListVec::iterator it2 = alist_.end();
	if (it != alist_.end())
	{
		int newrage = it->rage + a_rage;
		if (newrage < 0)
			newrage = 0;

		if (newrage)
		{
			it2 = it;
			if (a_rage > 0)
			{
				while (it2 != alist_.begin())
				{
					--it2;
					if (it2->rage > newrage)
						break;
				}

				AggroNode node = *it;
				if (it2->rage > newrage)
				{
					++it2;
				}
				else
				{
					ASSERT(it2 == alist_.begin());
				}

				for (; it != it2; --it)
				{
					*it = *(it - 1);
				}
				*it2 = node;
			}
			else
			{
				++it2;
				while (it2 != alist_.end())
				{
					if (it2->rage < newrage)
						break;
					++it2;
				}

				AggroNode node = *it;
				--it2;
				for (; it != it2; ++it)
				{
					*it = *(it + 1);
				}
				*it2 = node;
			}

			it2->rage = newrage;
			return it2 - alist_.begin();
		}
		else
		{
			alist_.erase(it);
			return it - alist_.begin();
		}
	}
	else
	{
		// 新加入的条目
		if (a_rage < 0)
			return 0;

		if (alist_.size() >= max_size_)
		{
			alist_.pop_back();
		}

		it2 = Find(a_rage);
		alist_.insert(it2, AggroNode(id, a_rage));
		return it2 - alist_.begin();
	}

	return 0;
}

int AggroList::AddToFirst(const XID& id, int addon_rage)
{
	ASSERT(addon_rage >= 0);

	AggroListVec::iterator it = Find(id);
	if (it == alist_.end())
	{
		if (alist_.size() == 0)
		{
			return AddRage(id, addon_rage);
		}
		else
		{
			return AddRage(id, alist_[0].rage + addon_rage);
		}
	}
	else
	{
		return AddRage(id, alist_[0].rage - it->rage + addon_rage);
	}
	
	return -1;
}

void AggroList::RemoveFirst()
{
	if (alist_.empty())
		return;

	alist_.erase(alist_.begin());
}
	
bool AggroList::GetFirst(XID& target) const
{
	if (alist_.empty())
		return false;

	target = alist_[0].id;
	return true;
}

void AggroList::Clear()
{
	alist_.clear();
}

int AggroList::Remove(const XID& id)
{
	AggroListVec::iterator it  = Find(id);
	if (it == alist_.end())
		return -1;
	return alist_.erase(it) - alist_.begin();
}

} // namespace gamed
