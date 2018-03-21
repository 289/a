#include <iostream>
#include <algorithm>
#include <assert.h>
#include "game_stl.h"
#include "map_astar.h"
#include "cluster.h"


namespace pathfinder
{

/********************************MAStar******************************/
/********************************MAStar******************************/
/********************************MAStar******************************/
/********************************MAStar******************************/

MAStar::MAStar() :
	_nodes_touched(0),
	_nodes_expanded(0),
	_max_expanded(50000)
{
}

MAStar::~MAStar()
{
	_nodes_touched  = 0;
	_nodes_expanded = 0;

	_openlist.Clear();
	_closelist.Clear();
}

int32_t MAStar::GetNodesExpanded() const
{
	return _nodes_expanded;
}

bool MAStar::BuildPath(const MapNode* dest, Path& path) const
{
	//path.clear();
	const MapNode* pNode = dest;
	if (!pNode)
	{
		return false;
	}

	Path tmp_path;
	for (;;)
	{
		tmp_path.push_back(pNode->GetCoord());
		if ((pNode = pNode->GetParent()) != NULL)
		{
			continue;
		}
		else
		{
			break;
		}
	}

	int i = tmp_path.size() - 1;
	while (i >= 0)
	{
		path.push_back(tmp_path[i]);
		-- i;
	}
	return true;
}

};
