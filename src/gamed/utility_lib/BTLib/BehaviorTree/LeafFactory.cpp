#include "LeafFactory.h"

#include "Integration/Nop.h"
#include "Integration/ConditionSuccessful.h"
#include "Integration/ConditionFailure.h"

#include "BTAssert.h"


///
/// register leaf node
///
#define REGISTER_LEAF(name, call) Register(std::string(name), call)


using namespace std;

namespace BTLib {

BTLeafFactory::BTLeafFactory()
{
	registerDefaultLeafNodes();
}

BTLeafFactory::~BTLeafFactory()
{
	Destroy();
}

void BTLeafFactory::Destroy()
{
	UnRegisterAll();
}

void BTLeafFactory::UnRegisterAll()
{
	entries_.clear();
}

void BTLeafFactory::Register(const string& name, CreateLeafCallback creator)
{
	if (entries_.insert(make_pair(name, creator)).second == false)
	{
		BT_ASSERT(false);
	}
}

void BTLeafFactory::UnRegister(string& name)
{
	LeafCBMap::iterator it = entries_.find(name);
	if (entries_.end() == it)
	{
		BT_ASSERT(false);
	}

	entries_.erase(it);
}

bool BTLeafFactory::IsRegistered(string& name) const
{
	LeafCBMap::const_iterator it = entries_.find(name);
	if (entries_.end() == it)
	{
		return false;
	}

	return true;
}

BTLeafFactory::CreateLeafCallback BTLeafFactory::GetCreator(string& name) const
{
	LeafCBMap::const_iterator it = entries_.find(name);
	if (entries_.end() == it)
	{
		BT_ASSERT(false);
		return NULL;
	}

	return it->second;
}

void BTLeafFactory::registerDefaultLeafNodes()
{
	//Register some classes here:
	REGISTER_LEAF("Nop", &createLeafNode<Nop>);
	REGISTER_LEAF("ConditionSuccessful", &createLeafNode<ConditionSuccessful>);
	REGISTER_LEAF("ConditionFailure", &createLeafNode<ConditionFailure>);
}

} // namespace BTLib
