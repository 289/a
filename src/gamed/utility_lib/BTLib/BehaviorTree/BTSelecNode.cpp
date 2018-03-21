#include "BTSelecNode.h"

#include "BehaviorTree.h"
#include "BTAssert.h"


namespace BTLib {

BTSelecNode::BTSelecNode(BehaviorTree* tree, BTNode* parent)
	: BTNode(tree, parent)
{
}

BTSelecNode::~BTSelecNode()
{
}

void BTSelecNode::init()
{
}
	
void BTSelecNode::update(NodeStatus result)
{
	tree_->setCurrentNode(id_/NODE_ID_TWO_DIGIT, id_%NODE_ID_TWO_DIGIT);

	if (result != EXECUTING)
	{
		++itChild_;
	}

	// FAILURE
	if (result == FAILURE)
	{
		// Try the next one, if I can.
		if (itChild_ != children_.end())
		{
			(*itChild_)->executeDbg();
		}
		else
		{
			// If not, Selector returns a FAILURE
			itChild_    = children_.begin();
			nodeStatus_ = FAILURE;
			parent_->update(nodeStatus_);
		}
	}
	else if (result == SUCCESS)
	{
		// With SUCCESS, everything is done, just notify SUCCESS to the parent.
		itChild_    = children_.begin();
		nodeStatus_ = SUCCESS;
		parent_->update(nodeStatus_);
	}
}

void BTSelecNode::execute()
{
	BT_ASSERT(children_.size() > 0);

	itChild_ = children_.begin();
	(*itChild_)->executeDbg();
}

} // namespace BTLib
