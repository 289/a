#ifndef GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREE_H_
#define GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREE_H_

#include <stdint.h>
#include <string>
#include <map>

#include "BTBlackboard.h"


namespace BTLib {

/* States of the behavior tree. */
enum BT_State
{   
	BTST_NONDEF,    /* BT obj is create, but not init */
	BTST_IDLE,      /* BT is ready to execute.  */
	BTST_EXECUTING, /* BT has been started executing. */
	BTST_CHANGING,  /* BT is being changed.  */

	BTST_MAXIMUM = 255
};  

class BTNode;
class BTLinkNode;
class BTSelecNode;
class BTSeqNode;

/**
 * The core BehaviorTree class.
 * It is responsible for holding the collection of nodes that comprises the BT (Behavior Tree)
 * and the functionality for executing an individual BT.
 */
class BehaviorTree
{
	friend class BTNode;
	friend class BTLinkNode;
	friend class BTSelecNode;
	friend class BTSeqNode;
public:
	BehaviorTree(const std::string& tree_name);
	~BehaviorTree();

	// **** thread unsafe **** //
	int    execute();
	void   addNode(BTNode* parent, uint8_t deep, BTNode* node, uint8_t& out_index);
	void   printTree();

	// GETTERS - thread unsafe //
	inline BT_State getBTState() const;
	BTNode*         getRootNode();
	inline const std::string& getTreeName() const;

	// SETTERS - thread unsafe //
	
	
	// TEMPLATE FUNCs - thread unsafe //
	template <typename T>
	bool    getBlackboard(T& data);
	template <typename T>
	void    setBlackboard(const T& data);
	inline void resetBlackboard();


protected:
	inline void     setCurrentNode(uint8_t deep, uint8_t index);


private:
	void    initTree();
	void    reset();
	void    printRecur(BTNode* node, int deep);


private:
	// RUNTIME CONST DATA //
	BTLinkNode*    root_;
	std::string    treeName_; // unique identification of a tree
	typedef std::map<uint8_t, BTNode*> IdMapNode;
	typedef std::map<uint8_t, IdMapNode > TreeNodes;
	TreeNodes      nodesOfTree_;
	bool           isInited_;

	// RUNTIME VARIABLE DATA //
	BTBlackboard   blackBoard_;
	BT_State       btState_;
	uint8_t        curDeep_;  // 当前执行到树的第几层
	uint8_t        curIndex_; // 当前执行层数的第几个节点
};

///
/// inline func 
///
inline BT_State BehaviorTree::getBTState() const
{
	return btState_;
}

inline const std::string& BehaviorTree::getTreeName() const
{
	return treeName_;
}

inline void BehaviorTree::setCurrentNode(uint8_t deep, uint8_t index)
{
	curDeep_  = deep;
	curIndex_ = index;
}

inline void BehaviorTree::resetBlackboard()
{
	blackBoard_.reset();
}

///
/// template func
///
template <typename T>
bool BehaviorTree::getBlackboard(T& data)
{
	return blackBoard_.getBlackboard(data);
}

template <typename T>
void BehaviorTree::setBlackboard(const T& data)
{
	blackBoard_.setBlackboard(data);
}

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREE_H_
