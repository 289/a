#include "BTSeqNode.h"

#include "BehaviorTree.h"
#include "BTAssert.h"


namespace BTLib {

BTSeqNode::BTSeqNode(BehaviorTree* tree, BTNode* parent)
	: BTNode(tree, parent)
{
}

BTSeqNode::~BTSeqNode()
{
}

void BTSeqNode::init()
{
}
	
void BTSeqNode::update(NodeStatus result)
{
	tree_->setCurrentNode(id_/NODE_ID_TWO_DIGIT, id_%NODE_ID_TWO_DIGIT);

	if (result != EXECUTING)
	{
		++itChild_;
	}

	// SUCCESS
	if (result == SUCCESS)
	{
		// If I can, go on with the next one...
		if (itChild_ != children_.end())
		{
			(*itChild_)->executeDbg();
		}
		else
		{
			// If not, we succeed! Notify that to my parent.
			itChild_    = children_.begin();
			nodeStatus_ = SUCCESS;
			parent_->update(nodeStatus_);
		}
	}
	else if (result == FAILURE)
	{
		itChild_    = children_.begin();
		nodeStatus_ = FAILURE;
		parent_->update(nodeStatus_);
	}
}

void BTSeqNode::execute()
{
	BT_ASSERT(children_.size() > 0);

	itChild_ = children_.begin();
	(*itChild_)->executeDbg();
}

} // namespace BTLib
