#include "area_btlib_link.h"

#include "area_player_leaf.h"

using namespace BTLib;

namespace gamed {

#define REGISTER_LEAF(name, call) \
	leaf_factory_.Register(std::string(name), call)

BTLib::BTLeafFactory AreaEvLibraryLink::leaf_factory_;
BTLib::LoadBehaviorTree AreaEvLibraryLink::bt_loader_; 

///
/// static member func
///
BTLib::BTLeafFactory* AreaEvLibraryLink::GetLeafFactory()
{
	return &leaf_factory_;
}

BTLib::LoadBehaviorTree* AreaEvLibraryLink::GetBTLoader()
{
	return &bt_loader_;
}

///
/// member func
///
bool AreaEvLibraryLink::Init()
{
	if (!BehaviorTreeLibrary::Init(&leaf_factory_))
		return false;

	// 需要在BehaviorTreeLibrary::Init()之后调用
	if (!bt_loader_.Init(&leaf_factory_))
		return false;

	return true;
}

void AreaEvLibraryLink::registerLeafNodes(BTLeafFactory* factory)
{
	REGISTER_LEAF("AreaPlayerCondition", &createLeafNode<AreaPlayerCondition>);
	REGISTER_LEAF("AreaPlayerAction", &createLeafNode<AreaPlayerAction>);
}

} // namespace gamed
