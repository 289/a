#include "BTNode.h"

#include "BehaviorTree.h"
#include "BTUtility.h"
#include "BTAssert.h"


namespace BTLib {

BTNode::BTNode(BehaviorTree* tree, BTNode* parent)
	: parent_(parent),
	  tree_(tree),
	  id_(-1),
	  nodeStatus_(START)
{
}

BTNode::~BTNode()
{
	ChildNodes::const_iterator i;
	for (i = children_.begin(); i != children_.end(); ++i)
	{
		BTNode* node = (*i);
		BT_DELETE(node);
	}
}

void BTNode::pushBack(BTNode* node)
{
	const ChildNodes::size_type t_size_before = children_.size();

	children_.push_back(node);

	const ChildNodes::size_type t_size_after = children_.size();
	BT_ASSERT(t_size_after > t_size_before);
}

void BTNode::initNodes()
{
	ChildNodes::const_iterator itChild = children_.begin();
	for (; itChild != children_.end(); ++itChild)
	{
		BTNode* child = (*itChild);
		BT_ASSERT(child);
		child->initNodes();

		// Init for each node
		child->init();
	}

	initRuntimeData();
}

// Resets the node and calls resetNodes() method on children nodes.
void BTNode::resetNodes()
{
	ChildNodes::const_iterator itChild = children_.begin();
	for (; itChild != children_.end(); ++itChild)
	{
		BTNode* child = (*itChild);
		BT_ASSERT(child);
		child->resetNodes();

		// reset for each node
		child->reset();
	}

	resetRuntimeData();
}

// Searches a node by its id and returns it if found.
BTNode* BTNode::searchNode(int id)
{
	ChildNodes::const_iterator itChild = children_.begin();
	for (; itChild != children_.end(); ++itChild)
	{
		BTNode* child = (*itChild);
		BT_ASSERT(child);
		if (child->getId() == id)
		{
			// FOUND!
			return child;
		}

		// search in this node then
		BTNode* n = child->searchNode(id);
		if (n) return n;
	}

	return NULL;
}

void BTNode::executeDbg()
{
	nodeStatus_ = EXECUTING;
	tree_->setCurrentNode(id_/NODE_ID_TWO_DIGIT, id_%NODE_ID_TWO_DIGIT);

	// log ?????? gdb
	execute();
}

void BTNode::initRuntimeData()
{
	nodeStatus_ = START;
	itChild_    = children_.end();
}

void BTNode::resetRuntimeData()
{ 
	nodeStatus_ = START;
	itChild_    = children_.end();
}

} // namespace BTLib
