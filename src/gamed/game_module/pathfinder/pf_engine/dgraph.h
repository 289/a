#ifndef __PF_DGRAPH_H__
#define __PF_DGRAPH_H__

#include <vector>
#include <stdint.h>
#include <assert.h>
#include "point.h"
#include "types.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace net
}; // namespace shared

namespace pathfinder
{

struct VexNode;
struct ArcNode;

/**
 * @class DGraph
 * @brief directed graph--有向图
 * @brief 采用十字链表法存储图数据
 */
class DGraph
{
public:
	DGraph();
	~DGraph();

	friend class GlobalTransDataGenerator;

	bool Load(shared::net::Buffer& buffer);
	void Save(shared::net::Buffer& buffer);

	int  AddVetex(VexNode* node);
	bool RmvVetex(VexNode* node);
	bool RmvVetex(int16_t owner, const PointI& pt);
	int  AddArc(ArcNode* node);
	bool RemoveArc(ArcNode* arc);

	const VexNode* GetNode(int index) const;

	//test
	void dump();

protected:
private:
	typedef std::vector<VexNode*> VexVec;
	typedef std::vector<ArcNode*> ArcVec;
	VexVec _vex_list;
	ArcVec _arc_list;
};


/**
 * @class ArcNode
 * @brief 弧结点
 * @brief 有向图弧的弧头和弧尾定义
 *        VexA----------------->VexB
 *        VexA为弧尾
 *        VexB为弧头
 */
struct ArcNode
{
	int16_t  num;        // 弧编号
	float    weight;     // 弧权重
	int16_t  tail_vex;   // 弧尾顶点的位置
	int16_t  head_vex;   // 弧头顶点的位置
	ArcNode* tail_link;  // 和本弧的弧尾相同的弧的链
	ArcNode* head_link;  // 和本弧的弧头相同的弧的链

	ArcNode():
		num(-1),
		weight(0.0f),
		tail_vex(-1),
		head_vex(-1),
		tail_link(NULL),
		head_link(NULL)
	{
	}

	ArcNode(float __weight, int16_t __tail_vex, int16_t __head_vex): num(-1)
	{
		weight    = __weight;
		tail_vex  = __tail_vex;
		head_vex  = __head_vex;
		tail_link = NULL;
		head_link = NULL;
	}

	~ArcNode()
	{
		weight    = 0;
		tail_vex  = 0;
		head_vex  = 0;
		tail_link = NULL;
		head_link = NULL;
	}

	bool operator == (const ArcNode& rhs) const
	{
		return weight == rhs.weight &&
			   tail_vex == rhs.tail_vex &&
			   head_vex == rhs.head_vex &&
			   tail_link == rhs.tail_link &&
			   head_link == rhs.head_link;
	}

	float GetWeight() const
	{
		return weight;
	}

	int32_t GetFrom() const  { return head_vex; }
	int32_t GetTo() const    { return tail_vex; }
	int32_t GetIndex() const { return num; }
};


/**
 * @class VexNode
 * @brief 顶点结点
 */
struct VexNode
{
	int16_t num;        // 顶点编号
	int16_t owner;      // 标记属于哪个簇或图
	int16_t x, z;       // 坐标值
	ArcNode* first_in;  // 指向该顶点的第一条入弧
	ArcNode* first_out; // 指向该顶点的第一条出弧

	VexNode() :
		num(-1),
		owner(0),
		x(0),
		z(0),
		first_in(NULL),
		first_out(NULL)
	{
	}

	VexNode(short __owner, short __x, short __z):
		num(-1),
		owner(__owner),
		x(__x),
		z(__z),
		first_in(NULL),
		first_out(NULL)
	{
	}

	VexNode(const VexNode& vex):
		num(vex.num),
		owner(vex.owner),
		x(vex.x),
		z(vex.z),
		first_in(vex.first_in),
		first_out(vex.first_out)
	{
	}

	~VexNode()
	{
		num = -1;
		owner = 0;
		x = 0;
		z = 0;
		first_in = NULL;
		first_out = NULL;
	}

	bool operator == (const VexNode& rhs) const
	{
		return owner == rhs.owner &&
			   x == rhs.x &&
			   z == rhs.z &&
			   first_in == rhs.first_in &&
			   first_out == rhs.first_out;
	}

	PointI GetCoord() const
	{
		return PointI(x, z);
	}

	std::vector<ArcNode*> GetNeighbours() const
	{
		std::vector<ArcNode*> list;
		ArcNode* arc = first_out;
		while (arc)
		{
			list.push_back(arc);
			arc = arc->tail_link;
		};
		return list;
	}

	int32_t GetIndex() const
	{
		return num;
	}

	int32_t GetOwner() const
	{
		return owner;
	}
};

};

#endif // __PF_DGRAPH_H__
