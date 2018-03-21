#include "BTLinkNode.h"

#include "BehaviorTree.h"
#include "BTAssert.h"


namespace BTLib {

BTLinkNode::BTLinkNode(BehaviorTree* tree, BTNode* parent)
	: BTNode(tree, parent)
{
}

BTLinkNode::~BTLinkNode()
{
}

void BTLinkNode::update(NodeStatus result)
{
	tree_->setCurrentNode(id_/NODE_ID_TWO_DIGIT, id_%NODE_ID_TWO_DIGIT);
}

void BTLinkNode::execute()
{
	// A link node should have 1 child
	BT_ASSERT(children_.size() <= 1);

	itChild_ = children_.begin();
	(*itChild_)->executeDbg();
}

} // namespace BTLib
