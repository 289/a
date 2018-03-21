#include "BehaviorTreeLibrary.h"

#include "BehaviorTree/LeafFactory.h"


namespace BTLib {

bool BehaviorTreeLibrary::Init(BTLeafFactory* factory)
{
	leafFactory_ = factory;
	registerLeafNodes(leafFactory_);
	return true;
}

void BehaviorTreeLibrary::Finalize()
{
	leafFactory_->Destroy();
}

} // namespace BTLib
