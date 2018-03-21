#include "BehaviorTree.h"

#include <stdio.h>
#include <string>

#include "BTAssert.h"
#include "BTNode.h"
#include "BTLinkNode.h"
#include "BTUtility.h"


namespace BTLib {

using namespace std;

namespace 
{
	const string V_line = "|    ";
	const string B_line = "|----";
} // namespace Anonymous

BehaviorTree::BehaviorTree(const std::string& tree_name)
	: root_(NULL),
	  treeName_(tree_name),
	  isInited_(false),
	  btState_(BTST_NONDEF),
	  curDeep_(0),
	  curIndex_(0)
{
	root_ = new BTLinkNode(this, NULL);
	nodesOfTree_[0][0] = root_;
}

BehaviorTree::~BehaviorTree()
{
	treeName_ = "";
	isInited_ = false;
	btState_  = BTST_NONDEF;
	curDeep_  = 0;
	curIndex_ = 0;
	BT_DELETE(root_); // This deletes every node in the Tree in cascade.
}

void BehaviorTree::initTree()
{
	root_->initNodes();
	isInited_ = true;
	btState_  = BTST_IDLE;
}

void BehaviorTree::addNode(BTNode* parent, uint8_t deep, BTNode* node, uint8_t& out_index)
{
	BT_ASSERT(node && !isInited_);
	int index = nodesOfTree_[deep].size();
	BT_ASSERT(NULL == nodesOfTree_[deep][index]);
	nodesOfTree_[deep][index] = node;
	parent->pushBack(node);

	out_index = index;
}

BTNode* BehaviorTree::getRootNode()
{
	return static_cast<BTNode*>(root_);
}

int BehaviorTree::execute()
{
	NodeStatus result = START;

	if (!isInited_)
	{
		initTree();
	}

	if (root_ && root_->getNumChildren() > 0 && BTST_IDLE == btState_)
	{
		btState_ = BTST_EXECUTING;

		BTNode* curNode = root_;
		if (curNode)
		{
			curNode->executeDbg();
			curNode = nodesOfTree_[curDeep_][curIndex_];
			result  = curNode->getNodeStatus();
		}

		reset();
	}

	return result;
}

void BehaviorTree::reset()
{
	root_->resetNodes();

	curDeep_    = 0;
	curIndex_   = 0;
	btState_    = BTST_IDLE;
	blackBoard_.reset();
}

void BehaviorTree::printTree()
{
	printRecur(root_, 0);
}

void BehaviorTree::printRecur(BTNode* node, int deep)
{
	for (int i = deep; i > 0; --i)
	{
		if (1 == i)
			printf("%s", B_line.c_str());
		else
			printf("%s", V_line.c_str());
	}

	printf("%04d\n", node->getId());
	for (size_t i = 0; i < node->children_.size(); ++i)
	{
		printRecur(node->children_[i], deep + 1);
	}
}

} // namespace BTLib
