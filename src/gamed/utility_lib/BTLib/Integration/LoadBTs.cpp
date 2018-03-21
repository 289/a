#include "LoadBTs.h"

#include <string.h>
#include <iostream>

#include "common/3rd/pugixml/inc/pugixml.hpp"

#include "BehaviorTree/BTAssert.h"
#include "BehaviorTree/BTUtility.h"
#include "BehaviorTree/BTNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTSelecNode.h"
#include "BehaviorTree/BTSeqNode.h"
#include "BehaviorTree/LeafFactory.h"


using namespace std;
using namespace pugi;

namespace BTLib {

typedef std::string runtime_error;

LoadBehaviorTree::LoadBehaviorTree()
	: isInited_(false),
	  leafFactory_(NULL)
{
}

LoadBehaviorTree::~LoadBehaviorTree()
{
	isInited_    = false;
	leafFactory_ = NULL;
}

bool LoadBehaviorTree::Init(const BTLeafFactory* factory)
{
	BT_ASSERT(false == isInited_);
	leafFactory_ = factory;
	isInited_    = true;
	return true;
}

BehaviorTree* LoadBehaviorTree::LoadFile(const std::string& fromXMLFile)
{
	BT_ASSERT(fromXMLFile.length() > 0);

	xml_document doc;
	xml_parse_result result = doc.load_file(fromXMLFile.c_str());
	if (!result) 
		return NULL;
	
	return Load(doc);
}

BehaviorTree* LoadBehaviorTree::LoadBuffer(const void* contents, size_t size)
{
	BT_ASSERT(NULL != contents && size > 0);

	xml_document doc;
	xml_parse_result result = doc.load_buffer(contents, size);
	if (!result) 
		return NULL;

	return Load(doc);
}

BehaviorTree* LoadBehaviorTree::Load(xml_document& doc)
{
	BT_ASSERT(isInited_);

	xml_node behaviortree = doc.child("BehaviorTree");
	if (behaviortree.empty()) 
		return NULL;

	std::string treename = behaviortree.attribute("Name").value();
	BehaviorTree* out_tree = new BehaviorTree(treename);
	if (NULL == out_tree)
		return NULL;

	xml_node element = behaviortree.child("Node");
	if (element.empty())
	{
		BT_DELETE(out_tree);
		return NULL;
	}

	try
	{
		loadRecur(element, out_tree, out_tree->getRootNode(), 0);
	}
	catch (char* str)
	{
		BT_DELETE(out_tree);
		cout << "Exception raised: " << str << endl;
	}

	return out_tree;
}

void LoadBehaviorTree::loadRecur(xml_node& a_element, 
		                         BehaviorTree* a_tree, 
								 BTNode* a_parentElement, 
								 int a_deep)
{
	BT_ASSERT(!a_element.empty());
	BT_ASSERT(a_tree);
	BT_ASSERT(a_parentElement);
	BT_ASSERT(a_deep >= 0);
	
	// Type
	BTNode* t_next_level_node  = NULL;
	const char* nodeType = a_element.attribute("Type").value();
	int nodeId           = a_element.attribute("Id").as_int();
	if (nodeType)
	{
		if (strcmp(nodeType, "Root") == 0)
		{
			t_next_level_node = a_parentElement;
			a_parentElement->setId(0);
			BT_ASSERT(nodeId == 0);
		}
		else if (strcmp(nodeType, "Selector") == 0)
		{
			// Now add a new selector node to the behavior tree
			BTSelecNode* t_new_node = new BTSelecNode(a_tree, a_parentElement);
			insertTreeNode(a_tree, a_parentElement, a_deep, t_new_node, nodeId);
			t_next_level_node = t_new_node;
		}
		else if (strcmp(nodeType, "Sequence") == 0)
		{
			// Now add a new sequence node to the behavior tree
			BTSeqNode* t_new_node = new BTSeqNode(a_tree, a_parentElement);
			insertTreeNode(a_tree, a_parentElement, a_deep, t_new_node, nodeId);
			t_next_level_node = t_new_node;
		}
		else if (strcmp(nodeType, "Filter") == 0)
		{
		}
		else if (strcmp(nodeType, "Condition") == 0 || strcmp(nodeType, "Action") == 0)
		{
			// Parameter "Operation"
			const char* op = a_element.attribute("Operation").value();
			if (op)
			{
				string operation(op);
				BT_ASSERT(leafFactory_->IsRegistered(operation));

				// Creation of node using factory
				ClassLeafNode* t_new_node = leafFactory_->GetCreator(operation)(a_tree, a_parentElement);
				const char* sn = a_element.attribute("ScriptName").value();
				const char* sf = a_element.attribute("ScriptFunc").value();
				if (sn && sf)
				{
					string scriptName(sn);
					t_new_node->setScriptName(scriptName);
					string scriptFunc(sf);
					t_new_node->setScriptFunc(scriptFunc);

					insertTreeNode(a_tree, a_parentElement, a_deep, t_new_node, nodeId);
				}
				else
				{
					throw(runtime_error("ScriptName or ScriptFunc attribute undefined in leaf node."));
				}

				// On leaf nodes, returns immedeately without trying to count any children further
				return;
			}
			else
			{
				throw(runtime_error("Operation attribute undefined in leaf node."));
			}
		}
		else
		{
			throw(runtime_error("Node type undefined."));
		}
	}

	// if there was no new node created, then assume the parent node is the next level's node
	if (t_next_level_node == NULL)
	{
		t_next_level_node = a_parentElement;
	}

	// Read attributes!
	// ????????
	
	// Lock for the connector!
	// ???????
	
	// Lets go for the children of this node.
	xml_node element = a_element.child("Node");
	BT_ASSERT(element && "ERR:内部节点当做叶子节点使用！");
	while (element)
	{
		// Read the node.
		loadRecur(element, a_tree, t_next_level_node, a_deep + 1);
		// Get the next sibling, if any.
		element = element.next_sibling("Node");
	}
}

void LoadBehaviorTree::insertTreeNode(BehaviorTree* a_tree, BTNode* a_parentElement, int a_deep, BTNode* a_new_node, int a_xml_nodeid)
{
	uint8_t t_cur_level_index = 0;

	a_tree->addNode(a_parentElement, a_deep, a_new_node, t_cur_level_index);
	int t_nodeId = a_deep * NODE_ID_TWO_DIGIT + t_cur_level_index;
	a_new_node->setId(t_nodeId);
	BT_ASSERT(a_xml_nodeid == t_nodeId);
}

} // namespace BTLib
