#ifndef __PF_GENERATOR_H__
#define __PF_GENERATOR_H__

#include <map>
#include <vector>
#include <stdint.h>
#include "point.h"
#include "types.h"

namespace pathfinder
{

class BitMap;
class MoveMap;
class DGraph;
class Cluster;
class TransTable;
class Generator
{
public:
	Generator(){}
	virtual ~Generator(){}
	virtual bool Load(const char* file_path) = 0;
	virtual bool Save(const char* file_path) = 0;
	virtual bool Process(const char* file_path=NULL) = 0;
};

/**
 * @class MapDataGenerator
 * @brief 生成本地图内寻路需要的数据
 * @brief 地编提供的地图数据基本都以米为单位，寻路模块使用的数据以格子为单位
 */
class MapDataGenerator : public Generator
{
protected:
private:
	int32_t    _mapid;
	int32_t    _map_length;
	int32_t    _map_width;
	BitMap*    _bmap;
	DGraph*    _graph;
	MoveMap*   _movemap;

private:
	/**
	 * 原始地编数据
	 */
	typedef int32_t Index;
	struct Area
	{
		struct TransPoint
		{
			PF2DVECTOR self_pos;
			PF2DVECTOR dest_pos;
			Index      dest_idx;
		};

		Index index;
		PF2DVECTOR origin;//相对位移
		float length;  //长度(单位:米)
		float width;   //宽度(单位:米)
		std::vector<TransPoint> tp_list;
	};

	struct AreaFinder
	{
		Index index;
		AreaFinder(Index idx): index(idx) {}
		bool operator () (const Area& area) const
		{
			return area.index == index;
		}
	};

	typedef std::vector<Area> AreaVec;
	AreaVec _area_list; // 区域列表

private:
	/**
	 * 在原始地编数据基础上处理得到的中间数据,
	 * 然后用这个中间数据来生成寻路模块需要的跨区域传送图
	 */
	struct Node
	{
		Index   cluster_idx;
		PointI  pos;
		PF2DVECTOR src_pos;
	};

	struct Edge
	{
		float weight;
		Index from, to;
	};

	struct ClusterX
	{
		PointI  origin;
		int32_t length;
		int32_t width;
		std::vector<int16_t> in_list;
		std::vector<int16_t> out_list;

		bool TestRange(const PointI& pt) const
		{
			PointI offset = pt - origin;
			return offset.x < length && offset.z < width;
		}
	};

	std::vector<Node>     _node_list;     // 图节点列表
	std::vector<Edge>     _edge_list;     // 图内边列表
	std::vector<ClusterX> _cluster_list;  // 簇列表

public:
	MapDataGenerator();
	virtual ~MapDataGenerator();

	virtual bool Load(const char* file_path);
	virtual bool Save(const char* file_path);
	virtual bool Process(const char* file_path=NULL);

protected:
private:

	/**
	 * 预处理地编数据的函数
	 */
	void GenerateAllNode();  //生成传送图节点
	void GenerateAllEdge();  //生成传送图的边
	void GenerateClusterX(); //生成区域(簇)

	/**
	 * 创建寻路模块数据的函数
	 */
	void CreateGraph();      //创建传送图
	void CreateMoveMap();    //创建通路图

	void dump();
};

/**
 * @class GlobalTransDataGenerator
 * @brief 生成跨地图寻路数据
 */
class GlobalTransDataGenerator : public Generator
{
private:
	/**
	 * 原始地编数据 
	 */
	typedef int32_t MapID;
	struct MapEntry
	{
		struct TransPoint
		{
			PF2DVECTOR self_pos;
			PF2DVECTOR dest_pos;
			MapID dest_mapid;
		};

		MapID mapid;
		std::vector<TransPoint> tp_list;
	};

	typedef std::vector<MapEntry> MapVec;
	MapVec _map_list;

private:
	typedef int32_t TransPointNum;
	typedef TransPointNum TPNum;
	struct MapX
	{
		std::vector<TPNum> out_list; //出口列表
		std::vector<TPNum> in_list;  //入口列表
	};

	struct Node
	{
		MapID  mapid;
		PointI coord;
	};

	struct Edge
	{
		TPNum from;
		TPNum to;
		float weight;
	};

	std::map<MapID, MoveMap*> _movemap_map;
	std::map<MapID, MapX> _mapx_map;
	std::vector<Node> _node_list;
	std::vector<Edge> _edge_list;

private:
	TransTable* _trans_table;
	DGraph*     _trans_graph;

public:
	GlobalTransDataGenerator();
	virtual ~GlobalTransDataGenerator();

	virtual bool Load(const char* file_path);
	virtual bool Save(const char* file_path);
	virtual bool Process(const char* file_path=NULL);

private:
	void LoadAllMoveMap(const char* filepath);
	void GenerateAllNodes();
	void GenerateAllEdges();

	void GenerateTransTable();
	void GenerateTransGraph();

	void dump();
};

};

#endif // __PF_GENERATOR_H__
