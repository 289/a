#include "ClassLeafNode.h"

#include "BTAssert.h"


namespace BTLib {

ClassLeafNode::ClassLeafNode(BTLeafType type, BehaviorTree* tree, BTNode* parent)
	: BTLeafNode(tree, parent),
	  leafType_(type)
{
}

ClassLeafNode::~ClassLeafNode()
{
	leafType_ = BTLEAF_UNDEF;
}

void ClassLeafNode::execute()
{
	// Step into LeafNode, and execution of the leaf node (Often performs lua script, in the derived class).
	nodeStatus_ = step();
	if (nodeStatus_ == SUCCESS || nodeStatus_ == FAILURE)
	{
		parent_->update(nodeStatus_);
	}
	else
	{
		BT_ASSERT(false);
	}
}

} // namespace BTLib
