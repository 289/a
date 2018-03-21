#ifndef GAMED_UTILITY_LIB_BTLIB_BTNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTNODE_H_

#include <stdint.h>
#include <vector>

#include "BehaviorTree/NonCopyable.h"


namespace BTLib {

#define NODE_ID_TWO_DIGIT 100

enum NodeStatus
{
	START     = 0,  /* The node is ready to start. */
	FAILURE   = 1,  /* The node returned failure. */
	SUCCESS   = 2,  /* The node returned success.  */
	EXECUTING = 3,  /* The node is currently executing. */
	
	STATUS_MAXIMUM = 255
};

class BTNode;
class BehaviorTree;

/* Vector of nodes. Used to represent the data structure that holds the children of this node. */
typedef std::vector<BTNode*> ChildNodes;

class BTNode : public BTLib::noncopyable
{
	friend class BehaviorTree;
public:
	BTNode(BehaviorTree* tree, BTNode* parent);
	virtual ~BTNode();

	/**
	 * @brief update 
	 *     Called when my child ends its execution
	 * @param result
	 *     Result of the child execution.
	 *     内部节点需要实现这个函数
	 */
	virtual void update(NodeStatus result) { }
	
	/**
	 * @brief initNodes
	 *     Initializes the node and calls initNodes() method on children nodes.
	 */
	void    initNodes();

	/**
	 * @brief resetNodes 
	 *     Resets the node and calls resetNodes() method on children nodes.
	 */
	void    resetNodes();

	/**
	 * @brief pushBack 
	 *     Adds a new node as a child of this.
	 */
	void    pushBack(BTNode* node);

	/**
	 * @brief searchNode 
	 *     Searches a node by its ID. The ID is given by the BT XML file.
	 * @param id
	 *     ID of the node to look for.
	 * @return 
	 *     the node if found, null if it was no found.
	 */
	BTNode* searchNode(int id);

	/**
	 * @brief executeDbg 
	 *     Manages breakpoint flags and logging, before
	 *     calling the virtual function execute() to execute this node.
	 */
	void    executeDbg();

	// GETTERS //
	BehaviorTree* getBehaviorTree() const { return tree_; }
	int           getId() const { return id_; }	
	NodeStatus    getNodeStatus() const { return nodeStatus_; }

	// SETTERS //
	void          setBehaviorTree(BehaviorTree* bt) { tree_ = bt; }
	void          setParent(BTNode* p) { parent_ = p; }
	void          setId(int16_t id) { id_ = id; }


protected:
	/**
	 * @brief execute 
	 *     This function is called when this node has to be executed.
	 */
	virtual void execute() { }

	/**
	 * @brief init 
	 *     This method is used to initialize the node 
	 *     (suggestion: read leaf node parameters here from property table)
	 */
	virtual void init() { }

	/**
	 * @brief reset 
	 *     This method is used to reset the values of the node to its initial state.
	 *     Every subclass must overwrite this method to reset its state (filter variables,
	 *     current child in selectors and conditions, etc...). 
	 *     This may be used too in class leaf nodes to reset user's leaves.
	 */
	virtual void reset() { }


private:
	void    resetRuntimeData();
	void    initRuntimeData();
	

protected:
	///
	// RUNTIME CONST DATA //
	///
	BTNode*       parent_;
	ChildNodes    children_;

	// Reference to the Behavior tree this node belongs.
	BehaviorTree* tree_;

	// ID of this node, given and used by the BT editor in a debug session.
	// 四位整数，前两位代表层数（deep），后两位代表在该层的第几个节点（index）
	// 比如：0803，表示该节点在树的第8层，第3个节点 - 从左往右
	int16_t       id_;

	
	///
	// RUNTIME VARIABLE DATA //
	///
	NodeStatus    nodeStatus_;
	ChildNodes::const_iterator itChild_;
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTNODE_H_
