#include <stdio.h>
#include "dgraph.h"
#include "utility.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/*****************************DGraph*********************************/
/*****************************DGraph*********************************/
/*****************************DGraph*********************************/
/*****************************DGraph*********************************/

DGraph::DGraph()
{
}

DGraph::~DGraph()
{
	for (size_t i = 0; i < _vex_list.size(); ++ i)
	{
		delete _vex_list[i];
	}

	for (size_t i = 0; i < _arc_list.size(); ++ i)
	{
		delete _arc_list[i];
	}

	_vex_list.clear();
	_arc_list.clear();
}

bool DGraph::Load(shared::net::Buffer& buffer)
{
	int32_t vex_count = buffer.ReadInt32();
	for (int i = 0; i < vex_count; ++ i)
	{
		int16_t owner  = buffer.ReadInt16();
		int16_t coordX = buffer.ReadInt16();
		int16_t coordZ = buffer.ReadInt16();
		VexNode* node = new VexNode(owner, coordX, coordZ);
		if (!node)
		{
			return false;
		}

		AddVetex(node);
	}

	int32_t arc_count = buffer.ReadInt32();
	for (int i = 0; i < arc_count; ++ i)
	{
		float weight = ReadFloatFromBuffer(buffer);
		int16_t tail_vex = buffer.ReadInt16();
		int16_t head_vex = buffer.ReadInt16();

		assert(head_vex >= 0 && head_vex < vex_count);
		assert(tail_vex >= 0 && tail_vex < vex_count);

		ArcNode* node = new ArcNode(weight, tail_vex, head_vex);
		if (!node)
		{
			return false;
		}

		AddArc(node);
	}
	return true;
}

void DGraph::Save(shared::net::Buffer& buffer)
{
	int32_t vex_count = _vex_list.size();
	buffer.AppendInt32(vex_count);
	for (int i = 0; i < vex_count; ++ i)
	{
		VexNode* vex = _vex_list[i];
		buffer.AppendInt16(vex->owner);
		buffer.AppendInt16(vex->x);
		buffer.AppendInt16(vex->z);
	}

	int32_t arc_count = _arc_list.size();
	buffer.AppendInt32(arc_count);
	for (int i = 0; i < arc_count; ++ i)
	{
		ArcNode* arc = _arc_list[i];
		buffer.Append(&(arc->weight), sizeof(float));
		buffer.AppendInt16(arc->tail_vex);
		buffer.AppendInt16(arc->head_vex);
	}
}

int DGraph::AddVetex(VexNode* node)
{
	if (node)
	{
		node->num = _vex_list.size();
		_vex_list.push_back(node);
		return node->num;
	}
	return -1;
}

bool DGraph::RmvVetex(VexNode* node)
{
	if (!node) return false;

	ArcNode* p = NULL;
	ArcNode* q = NULL;

	//删除以该点为弧头的所有弧
	p = node->first_in;
	while (p)
	{
		q = p->head_link;
		RemoveArc(p);
		p = q;
	};

	//删除以该点为弧尾的所有弧
	p = node->first_out;
	q = NULL;
	while (p)
	{
		q = p->tail_link;
		RemoveArc(p);
		p = q;
	};

	//被删除的顶点必须是最后一个顶点，否则禁止删除,
	//因为顶点的num字段标识其在数组中的位置，中间顶点被删除将导致顶点的num编号和其实际位置不一致，
	//并且弧里面还保存了这样的错误编号，所以程序运行将会造成致命错误.
	if (node == _vex_list.back())
	{
		_vex_list.pop_back();
		return true;
	}
	else
	{
		assert(false);
		return false;
	}
}

struct VexFinder
{
	int16_t owner;
	int16_t x, z;
	VexFinder(int16_t __id, int16_t __x, int16_t __z):
		owner(__id),
		x(__x),
		z(__z)
	{
	}

	bool operator() (const VexNode* vex) const
	{
		return vex->owner == owner && vex->x == x && vex->z == z;
	}
};

bool DGraph::RmvVetex(int16_t owner, const PointI& pt)
{
	VexVec::iterator it = std::find_if(_vex_list.begin(), _vex_list.end(), VexFinder(owner, pt.x, pt.z));
	if (it == _vex_list.end())
	{
		assert(false);
		return false;
	}

	return RmvVetex(*it);
}

int DGraph::AddArc(ArcNode* arc)
{
	if (arc)
	{
		arc->num       = _arc_list.size();
		arc->head_link = _vex_list[arc->head_vex]->first_in;
		arc->tail_link = _vex_list[arc->tail_vex]->first_out;
		_arc_list.push_back(arc);

		_vex_list[arc->head_vex]->first_in  = arc;
		_vex_list[arc->tail_vex]->first_out = arc;

		return arc->num;
	}
	return -1;
}

bool DGraph::RemoveArc(ArcNode* arc)
{
	if (!arc) return false;

	assert(_vex_list[arc->head_vex]->first_in);
	assert(_vex_list[arc->tail_vex]->first_out);

	ArcNode* p = NULL;
	ArcNode* q = NULL;

	//删除弧头结点中相应的入弧
	p = _vex_list[arc->head_vex]->first_in;
	q = p;
	if (p == arc)
	{
		_vex_list[arc->head_vex]->first_in = p->head_link;
	}
	else
	{
		while (p && p != arc)
		{
			q = p;
			p = p->head_link;
		};

		assert(p && p == arc);
		q->head_link = p->head_link;
	}

	p = q = NULL;

	//删除弧尾结点中相应的出弧
	p = _vex_list[arc->tail_vex]->first_out;
	q = p;
	if (p == arc)
	{
		_vex_list[arc->tail_vex]->first_out = p->tail_link;
	}
	else
	{
		while (p && p != arc)
		{
			q = p;
			p = p->tail_link;
		};

		assert(p && p == arc);
		q->tail_link = p->tail_link;
	}
	
	//删除弧
	if (arc == _arc_list.back())
	{
		_arc_list.pop_back();
	}
	else
	{
		ArcNode* tmp = _arc_list.back();
		tmp->num = arc->num;
		_arc_list[arc->num] = tmp;
		_arc_list.pop_back();
	}

	delete arc;
	return true;
}

const VexNode* DGraph::GetNode(int index) const
{
	if (index < 0 || index >= (int)(_vex_list.size()))
	{
		assert(false);
		return NULL;
	}
	return _vex_list[index];
}

//test
#if defined(_WINDOWS)
void DGraph::dump()
{
	for (size_t i = 0; i < _arc_list.size(); ++ i)
	{
		ArcNode* arc = _arc_list[i];
		fprintf(stdout, "Arc%ld(num=%d,weight=%f,from=%d,to=%d)\n", i, arc->num, arc->weight, arc->tail_vex, arc->head_vex);
	}

	for (size_t i = 0; i < _vex_list.size(); ++ i)
	{
		VexNode* node = _vex_list[i];
		fprintf(stdout, "vetex%ld(num=%d,mapid=%d,x=%d,z=%d,以该点为弧头的弧(", i, node->num, node->owner, node->x, node->z);

		ArcNode* arc = node->first_in;
		while (arc)
		{
			fprintf(stdout, "%d,", arc->num);
			arc = arc->head_link;
		};
		fprintf(stdout, "), 以该点位弧尾的弧(");

		arc = node->first_out;
		while (arc)
		{
			fprintf(stdout, "%d,", arc->num);
			arc = arc->tail_link;
		};
		fprintf(stdout, ")\n");
	}
}
#endif // #if defined(_WINDOWS)
};
