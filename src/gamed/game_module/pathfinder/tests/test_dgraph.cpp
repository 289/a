#include <vector>
#include "ifile.h"
#include "point.h"
#include "dgraph.h"

struct node
{
	int16_t mapid;
	int16_t x,z;
	node(int16_t __mapid, int16_t __x, int16_t __z):
		mapid(__mapid),
		x(__x),
		z(__z)
	{
	}
};

struct edge
{
	float weight;
	int16_t from, to;
	edge(float __weight, int16_t __from, int16_t __to):
		weight(__weight),
		from(__from),
		to(__to)
	{
	}
};

using namespace pathfinder;

void GenDGraphData();

int main()
{
	//generate data
	GenDGraphData();

	//load dgraph-data
	shared::net::Buffer buffer;
	if (!LoadFile("dgraph.data", buffer))
	{
		assert(false);
		return -1;
	}

	DGraph* graph = new DGraph();
	if (!graph || !graph->Load(buffer))
		return -1;

	fprintf(stdout, "原始有向图信息:\n");
	graph->dump();

	graph->RmvVetex(1, PointI(4,4));
	/*
	graph->RmvVetex(1, PointI(1,1));
	graph->RmvVetex(1, PointI(2,2));
	graph->RmvVetex(1, PointI(3,3));
	graph->RmvVetex(1, PointI(4,4));
	*/

	fprintf(stdout, "修改后有向图信息:\n");
	graph->dump();

	delete graph;
	return 0;
}

void GenDGraphData()
{
	std::vector<node> node_list;
	std::vector<edge> edge_list;

	node_list.push_back(node(1,1,1));
	node_list.push_back(node(1,2,2));
	node_list.push_back(node(1,3,3));
	node_list.push_back(node(1,4,4));

	edge_list.push_back(edge(1.0f, 0, 1));
	edge_list.push_back(edge(1.0f, 0, 2));
	edge_list.push_back(edge(1.0f, 2, 0));
	edge_list.push_back(edge(1.0f, 2, 3));
	edge_list.push_back(edge(1.0f, 3, 0));
	edge_list.push_back(edge(1.0f, 3, 1));
	edge_list.push_back(edge(1.0f, 3, 2));

	shared::net::Buffer buffer;

	int32_t node_count = node_list.size();
	buffer.AppendInt32(node_count);
	for (size_t i = 0; i < node_list.size(); ++ i)
	{
		node& __node = node_list[i];
		buffer.AppendInt16(__node.mapid);
		buffer.AppendInt16(__node.x);
		buffer.AppendInt16(__node.z);
	}

	int32_t edge_count = edge_list.size();
	buffer.AppendInt32(edge_count);
	for (size_t i = 0; i < edge_list.size(); ++ i)
	{
		edge& __edge = edge_list[i];
		buffer.AppendInt32(__edge.weight);
		buffer.AppendInt16(__edge.from);
		buffer.AppendInt16(__edge.to);
	}

	pathfinder::IFile ifile;
	if (!ifile.Open("dgraph.data", IFILE_WRITE | IFILE_BIN))
		return;

	size_t bytes = ifile.Write(buffer.peek(), buffer.ReadableBytes());
	if (bytes != buffer.ReadableBytes())
	{
		ifile.Close();
		return;
	}

	ifile.Close();
}
