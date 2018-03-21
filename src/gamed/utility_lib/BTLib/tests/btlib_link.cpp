#include "btlib_link.h"

#include "BTLib/Integration/ConditionSuccessful.h"
#include "BTLib/Integration/Nop.h"

using namespace BTLib;

#define REGISTER_LEAF(name, call) \
	leaf_factory_.Register(std::string(name), call)

BTLib::BTLeafFactory BTLibraryLink::leaf_factory_;

///
/// static member func
///
BTLib::BTLeafFactory* BTLibraryLink::GetLeafFactory()
{
	return &leaf_factory_;
}

///
/// member func
///
bool BTLibraryLink::Init()
{
	return BehaviorTreeLibrary::Init(&leaf_factory_);
}

void BTLibraryLink::registerLeafNodes(BTLeafFactory* factory)
{
	REGISTER_LEAF("AreaPlayerCondition", &createLeafNode<BTLib::ConditionSuccessful>);
	REGISTER_LEAF("AreaPlayerAction", &createLeafNode<BTLib::Nop>);
}

