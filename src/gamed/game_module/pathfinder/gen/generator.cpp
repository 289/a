#include "generator.h"
#include "ifile.h"
#include "game_stl.h"
#include "bit_map.h"
#include "graph.h"
#include "dgraph.h"
#include "move_map.h"
#include "path_track.h"
#include "cluster.h"
#include "global_router.h"
#include "pf_common.h"
#include "pf_engine.h"
#include "map_astar.h"
#include "graph_astar.h"
#include "utility.h"


namespace pathfinder
{

namespace {

    void pf_logprint(const char* file, int line, const char* expr)
    {
        char szTemp[1024] = {0};
        snprintf(szTemp, sizeof(szTemp), "[%s:%d] - %s\n", file, line, expr);
    }

#define PF_LOGPRINT(expr) \
    pf_logprint(__FILE__, __LINE__, expr);

#define PRINT_NODE_PAIR(node_from, node_to) \
    { \
        PF2DVECTOR from = __movemap->TransMap2World(node_from.coord); \
        PF2DVECTOR to   = __movemap->TransMap2World(node_to.coord); \
        fprintf(stderr, "[%s:%d] A* search failed, from:%d (%f,%f) -> to:%d (%f,%f)\n", \
                __FILE__, __LINE__, node_from.mapid, from.x, from.z, node_to.mapid, to.x, to.z); \
    }


} // Anonymous

/*************************************MapDataGenerator******************************/
/*************************************MapDataGenerator******************************/
/*************************************MapDataGenerator******************************/
/*************************************MapDataGenerator******************************/

MapDataGenerator::MapDataGenerator():
	_bmap(NULL),
	_graph(NULL),
	_movemap(NULL)
{
	_mapid      = 0;
	_map_length = 0;
	_map_width  = 0;
}

MapDataGenerator::~MapDataGenerator()
{
	if (_movemap)
	{
		delete _movemap;
	}

	_bmap  = NULL;
	_graph = NULL;
	_movemap = NULL;

	_area_list.clear();
	_node_list.clear();
	_edge_list.clear();
	_cluster_list.clear();
}

bool MapDataGenerator::Load(const char* file_path)
{
	IFile ifile;
	if (!ifile.Open(file_path, IFILE_READ | IFILE_BIN))
	{
		return false;
	}

	int32_t __mapid      = 0;
	int32_t __map_length = 0;
	int32_t __map_width  = 0;
	int32_t __grid_count = 0;
	float   __grid_size  = 0.0f;
	PF2DVECTOR __origin;

	ifile.Read(&__mapid, sizeof(int32_t));
	ifile.Read(&__map_length, sizeof(int32_t));
	ifile.Read(&__map_width, sizeof(int32_t));
	ifile.Read(&__grid_size, sizeof(float));
	ifile.Read(&(__origin.x), sizeof(float));
	ifile.Read(&(__origin.z), sizeof(float));

	int32_t __len        = 0;
	unsigned char* __buf = NULL;

	__grid_count = __map_length * __map_width;
	__len = __grid_count >> 3;
	if (__grid_count & 0x07)
	{
		__len += 1;
	}

	__buf = (unsigned char*)malloc(__len);
	memset(__buf, 0, __len);
	int bytes = ifile.Read(__buf, __len);
	if (bytes != __len)
	{
		assert(false);
		return false;
	}

	BitMap* __bmap = new BitMap();
	if (!__bmap)
		return false;

	__bmap->_map_id       = __mapid;
	__bmap->_map_length   = __map_length;
	__bmap->_map_width    = __map_width;
	__bmap->_grid_size    = __grid_size;
	__bmap->_buf_len      = __len;
	__bmap->_pBuf         = __buf;
	__bmap->_origin       = __origin;

	_bmap       = __bmap;
	_mapid      = __mapid;
	_map_length = __map_length;
	_map_width  = __map_width;

	//load all area
	int32_t cluster_count = 0;
	ifile.Read(&cluster_count, sizeof(int32_t));
	_area_list.resize(cluster_count);

	for (int i = 0; i < cluster_count; ++ i)
	{
		Area& area = _area_list[i];
		ifile.Read(&(area.index), sizeof(int32_t));
		ifile.Read(&(area.origin.x), sizeof(float));
		ifile.Read(&(area.origin.z), sizeof(float));
		ifile.Read(&(area.length), sizeof(float));
		ifile.Read(&(area.width), sizeof(float));

		int32_t tp_count = 0;
		ifile.Read(&tp_count, sizeof(int32_t));
		area.tp_list.resize(tp_count);
		for (int j = 0; j < tp_count; ++ j)
		{
			Area::TransPoint& tp = area.tp_list[j];
			ifile.Read(&(tp.self_pos.x), sizeof(float));
			ifile.Read(&(tp.self_pos.z), sizeof(float));
			ifile.Read(&(tp.dest_idx), sizeof(int32_t));
			ifile.Read(&(tp.dest_pos.x), sizeof(float));
			ifile.Read(&(tp.dest_pos.z), sizeof(float));
		}
	}

	__bmap->_buf.resize(__grid_count);
	for (int i = 0; i < __grid_count; ++ i)
	{
		__bmap->_buf[i] = __bmap->TestBit(i) ? '1' : '0';
	}

	ifile.Close();
	return true;
}

bool MapDataGenerator::Save(const char* file_path)
{
	IFile ifile;
	if (!ifile.Open(file_path, IFILE_WRITE | IFILE_BIN))
	{
		return false;
	}

	shared::net::Buffer buffer;
	_movemap->Save(buffer);

	size_t bytes = ifile.Write(buffer.peek(), buffer.ReadableBytes());
	if (bytes != buffer.ReadableBytes())
	{
		ifile.Close();
		return false;
	}

	ifile.Close();
	return true;
}

bool MapDataGenerator::Process(const char* filepath)
{
	GenerateClusterX();
	GenerateAllNode();
	GenerateAllEdge();

	CreateGraph();
	CreateMoveMap();

	//test
	//dump();

	return true;
}

void MapDataGenerator::GenerateClusterX()
{
	/**
	 * 产生簇原点,长度,宽度
	 */

	BitMap* __bmap = _bmap;
	assert(__bmap);

	float  __grid_size = __bmap->_grid_size;
	assert(__grid_size > 0.0f);

	PointI  __origin;
	int32_t __area_length   = 0;
	int32_t __area_width    = 0;

	_cluster_list.resize(_area_list.size());

	for (size_t i = 0; i < _area_list.size(); ++ i)
	{
		Area& area    = _area_list[i];
		__origin      = POINTF_2_POINTI(__bmap->TransWorld2Map(area.origin));
		__area_length = (int)((area.length / __grid_size) + 0.1f);
		__area_width  = (int)((area.width  / __grid_size) + 0.1f);

		ClusterX& cluster = _cluster_list[i];
		cluster.origin    = __origin;
		cluster.length    = __area_length;
		cluster.width     = __area_width;
	}
}

void MapDataGenerator::GenerateAllNode()
{
	/**
	 * 产生所有节点
	 * 产生簇内节点信息
	 * 产生跨簇路径
	 */

	BitMap* __bmap = _bmap;
	assert(__bmap);

	int32_t __dest_area_idx = 0;

	for (size_t i = 0; i < _area_list.size(); ++ i)
	{
		Area& area        = _area_list[i];
		ClusterX& cluster = _cluster_list[i];

		for (size_t j = 0; j < area.tp_list.size(); ++ j)
		{
			Area::TransPoint& tp = area.tp_list[j];

			AreaVec::iterator it = std::find_if(_area_list.begin(), _area_list.end(), AreaFinder(tp.dest_idx));
			assert(it != _area_list.end());
			__dest_area_idx = it - _area_list.begin();

			Node __node_from;
			Node __node_to;

			__node_from.cluster_idx = i;
			__node_from.src_pos = tp.self_pos;
			__node_from.pos = POINTF_2_POINTI(__bmap->TransWorld2Map(tp.self_pos));

			__node_to.cluster_idx = __dest_area_idx;
			__node_to.src_pos = tp.dest_pos;
			__node_to.pos = POINTF_2_POINTI(__bmap->TransWorld2Map(tp.dest_pos));

            if (!__bmap->IsPassable(__node_from.pos))
            {
                //起点不可达
                fprintf(stderr, "区域传送点的起点不可达, mapid:%d, 传送点坐标(%f,%f), 目标点坐标(%f,%f)\n", _mapid, tp.self_pos.x, tp.self_pos.z, tp.dest_pos.x, tp.dest_pos.z);
                assert(false);
            }

            if (!__bmap->IsPassable(__node_to.pos))
            {
                //终点不可达
                fprintf(stderr, "区域传送点的目标点不可达, mapid:%d, 传送点坐标(%f,%f), 目标点坐标(%f,%f)\n", _mapid, tp.self_pos.x, tp.self_pos.z, tp.dest_pos.x, tp.dest_pos.z);
                assert(false);
            }

			_node_list.push_back(__node_from);
			_node_list.push_back(__node_to);

			int32_t __node_index_from = _node_list.size() - 2;
			int32_t __node_index_to   = _node_list.size() - 1;

			Edge __edge;
			__edge.weight = 1.0f;
			__edge.from   = __node_index_from;
			__edge.to     = __node_index_to;
			_edge_list.push_back(__edge);

			cluster.out_list.push_back(__node_index_from);
			_cluster_list[__dest_area_idx].in_list.push_back(__node_index_to);
		}
	}
}

void MapDataGenerator::GenerateAllEdge()
{
	/**
	 * 生成每个簇的内部路径
	 */

	for (size_t i = 0; i < _cluster_list.size(); ++ i)
	{
		ClusterX& cluster = _cluster_list[i];

		std::vector<int16_t>& outlist = cluster.out_list;
		std::vector<int16_t>& inlist  = cluster.in_list;

		PointI __source;
		PointI __dest;
		Index  __source_idx = 0;
		Index  __dest_idx = 0;

		for (size_t j = 0; j < inlist.size(); ++ j)
		{
			__source_idx = inlist[j];
			__source = _node_list[__source_idx].pos;

			for (size_t k = 0; k < outlist.size(); ++ k)
			{
				__dest_idx = outlist[k];
				__dest = _node_list[__dest_idx].pos;

				Path __path;
				MAStar mAstar;
				if (mAstar.Search(_bmap, __source, __dest, __path, &cluster))
				{
					Edge __edge;
					__edge.from   = __source_idx;
					__edge.to     = __dest_idx;
					__edge.weight = Distance(__path);

					_edge_list.push_back(__edge);
				}
			}
		}
	}
}

void MapDataGenerator::CreateGraph()
{
	DGraph* __graph = new DGraph();
	if (!__graph)
		return;

	for (size_t i = 0; i < _node_list.size(); ++ i)
	{
		Node& node = _node_list[i];
		VexNode* __node = new VexNode(node.cluster_idx, node.pos.x, node.pos.z);
		if (!__node)
			return;

		__graph->AddVetex(__node);
	}

	for (size_t i = 0; i < _edge_list.size(); ++ i)
	{
		Edge& edge = _edge_list[i];
		ArcNode* __edge = new ArcNode(edge.weight, edge.from, edge.to);
		if (!__edge)
			return;

		__graph->AddArc(__edge);
	}

	_graph = __graph;
}

void MapDataGenerator::CreateMoveMap()
{
	MoveMap* __movemap = new MoveMap();
	if (!__movemap)
		return;

	std::vector<Cluster*> __cluster_list;
	__cluster_list.resize(_cluster_list.size());

	for (size_t i = 0; i < _cluster_list.size(); ++ i)
	{
		ClusterX& cluster = _cluster_list[i];

		Cluster* __cluster = new Cluster();
		if (!__cluster)
			return;

		__cluster->_base_pos  = cluster.origin;
		__cluster->_length   = cluster.length;
		__cluster->_width    = cluster.width;
		__cluster->_index    = i;
		__cluster->_in_list  = cluster.in_list;
		__cluster->_out_list = cluster.out_list;

		__cluster_list[i] = __cluster;
	}

	__movemap->_bmap     = _bmap;
	__movemap->_graph    = _graph;
	__movemap->_clusters = __cluster_list;
	_movemap             = __movemap;
}

void MapDataGenerator::dump()
{
	fprintf(stdout, "_node_list(%ld)\n", _node_list.size());
	for (size_t i = 0; i < _node_list.size(); ++ i)
	{
		Node& __node = _node_list[i];
		fprintf(stdout, "node%ld(cluster_idx=%d,pos(%d,%d),srcpos(%f,%f))\n", i, __node.cluster_idx, __node.pos.x, __node.pos.z, __node.src_pos.x, __node.src_pos.z);
	}

	fprintf(stdout, "_edge_list(%ld)\n", _edge_list.size());
	for (size_t i = 0; i < _edge_list.size(); ++ i)
	{
		Edge& __edge = _edge_list[i];
		fprintf(stdout, "edge%ld(from=%d(%d,%d),to=%d(%d,%d),weight=%f),", i, __edge.from, _node_list[__edge.from].pos.x, _node_list[__edge.from].pos.z, __edge.to, _node_list[__edge.to].pos.x, _node_list[__edge.to].pos.z, __edge.weight);
		fprintf(stdout, ")\n");
	}

	fprintf(stdout, "_cluster_list(%ld)\n", _cluster_list.size());
	for (size_t i = 0; i < _cluster_list.size(); ++ i)
	{
		ClusterX& __cluster = _cluster_list[i];
		fprintf(stdout, "cluster%ld(origin=(%d,%d), ", i, __cluster.origin.x, __cluster.origin.z);
		fprintf(stdout, "in_list(");
		for (size_t j = 0; j < __cluster.in_list.size(); ++ j)
			fprintf(stdout, "%d,", __cluster.in_list[j]);
		fprintf(stdout, "), ");
		fprintf(stdout, "out_list(");
		for (size_t j = 0; j < __cluster.out_list.size(); ++ j)
			fprintf(stdout, "%d,", __cluster.out_list[j]);
		fprintf(stdout, "))\n");
	}
}

/********************************GlobalTransDataGenerator******************************/
/********************************GlobalTransDataGenerator******************************/
/********************************GlobalTransDataGenerator******************************/
/********************************GlobalTransDataGenerator******************************/

GlobalTransDataGenerator::GlobalTransDataGenerator():
	_trans_table(NULL),
	_trans_graph(NULL)
{
}

GlobalTransDataGenerator::~GlobalTransDataGenerator()
{
	std::map<MapID, MoveMap*>::iterator it = _movemap_map.begin();
	for (; it != _movemap_map.end(); ++ it)
		delete it->second;

	if (_trans_table)
	{
		delete _trans_table;
		_trans_table = NULL;
	}

	if (_trans_graph)
	{
		delete _trans_graph;
		_trans_graph = NULL;
	}

	_map_list.clear();
	_mapx_map.clear();
	_movemap_map.clear();
	_node_list.clear();
	_edge_list.clear();
}

template <class T>
struct MapFinder
{
	int32_t mapid;
	MapFinder(int32_t __mapid): mapid(__mapid) {}
	bool operator () (const T& obj) const
	{
		return obj.mapid == mapid;
	}
};

template<class T>
struct TPFinder/*trans-point-finder*/
{
	PF2DVECTOR pos;
	TPFinder(const PF2DVECTOR& __pos): pos(__pos) {}
	bool operator () (const T& obj) const
	{
		return pos == obj.self_pos;
	}
};

bool GlobalTransDataGenerator::Load(const char* file_path)
{
	IFile ifile;
	if (!ifile.Open(file_path, IFILE_READ | IFILE_BIN))
	{
		return false;
	}

	int32_t map_count = 0;
	ifile.Read(&map_count ,sizeof(int32_t));
	_map_list.resize(map_count);

	for (int i = 0; i < map_count; ++ i)
	{
		int32_t __mapid =0;
		ifile.Read(&__mapid, sizeof(int32_t));
		MapVec::iterator map_it = std::find_if(_map_list.begin(), _map_list.end(), MapFinder<MapEntry>(__mapid));
		assert(map_it == _map_list.end());

		MapEntry& entry = _map_list[i];
		entry.mapid = __mapid;

		int32_t tp_count = 0;
		ifile.Read(&tp_count, sizeof(int32_t));
		entry.tp_list.resize(tp_count);

		for (int j = 0; j < tp_count; ++ j)
		{
			PF2DVECTOR __src_pos;
			ifile.Read(&(__src_pos.x), sizeof(float));
			ifile.Read(&(__src_pos.z), sizeof(float));

			std::vector<MapEntry::TransPoint>::iterator tp_it = std::find_if(entry.tp_list.begin(), entry.tp_list.end(), TPFinder<MapEntry::TransPoint>(__src_pos));
			assert(tp_it == entry.tp_list.end());

			MapEntry::TransPoint& tp = entry.tp_list[j];
			tp.self_pos = __src_pos;

			ifile.Read(&(tp.dest_mapid), sizeof(int32_t));
			ifile.Read(&(tp.dest_pos.x), sizeof(float));
			ifile.Read(&(tp.dest_pos.z), sizeof(float));
		}
	}

	ifile.Close();
	return true;
}

bool GlobalTransDataGenerator::Save(const char* file_path)
{
	IFile ifile;
	if (!ifile.Open(file_path, IFILE_WRITE | IFILE_BIN))
	{
		return false;
	}

	shared::net::Buffer buffer;
	_trans_table->Save(buffer);
	_trans_graph->Save(buffer);

	size_t bytes = ifile.Write(buffer.peek(), buffer.ReadableBytes());
	if (bytes != buffer.ReadableBytes())
	{
		ifile.Close();
		return false;
	}

	ifile.Close();
	return true;
}

bool GlobalTransDataGenerator::Process(const char* filepath)
{
	LoadAllMoveMap(filepath);
	GenerateAllNodes();
	GenerateAllEdges();

	//test
	//dump();

	GenerateTransTable();
	GenerateTransGraph();
	return true;
}

///
/// load file to Buffer
///
static bool LoadFile(const char* file_path, shared::net::Buffer& buffer)
{
	if (!file_path)
	{
		return false;
	}

	FILE* fp = fopen(file_path, "rb");
	if (!fp)
	{
		return false;
	}

	size_t file_size = 0;
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	buffer.EnsureWritableBytes(file_size);

	size_t bytes = fread(buffer.BeginWrite(), file_size, 1, fp);
	if (bytes != 1)
	{
		return false;
	}
	buffer.HasWritten(file_size);
	return true;
}

void GlobalTransDataGenerator::LoadAllMoveMap(const char* filepath)
{
	std::string full_path(filepath);
	full_path.append("movemap_");

	for (size_t i = 0; i < _map_list.size(); ++ i)
	{
		char __mapid[20];
		sprintf(__mapid, "%d", _map_list[i].mapid);

		std::string __full_path(full_path);
		__full_path.append(__mapid);
		__full_path.append(".gbd");

		shared::net::Buffer buffer;
		if (!LoadFile(__full_path.c_str(), buffer))
		{
			assert(false);
			return;
		}

		MoveMap* __movemap = new MoveMap();
		if (!__movemap)
			return;

		if (!__movemap->Load(buffer) || (__movemap->GetMapID() != _map_list[i].mapid))
		{
			delete __movemap;
			return;
		}

		_movemap_map[_map_list[i].mapid] = __movemap;
	}
}

void GlobalTransDataGenerator::GenerateAllNodes()
{
	/**
	 * 产生全局图上所有的点，包括传送点和入口点.
	 * 产生跨地图路径，跨地图路径的长度统一定义为1.0f
	 * 产生同一地图内的出口点和入口点
	 */

	for (size_t i = 0; i < _map_list.size(); ++ i)
	{
		MapEntry& __map = _map_list[i];

		std::vector<MapEntry::TransPoint>& __tp_list = __map.tp_list;
		for (size_t i = 0; i < __tp_list.size(); ++ i)
		{
			MapEntry::TransPoint& __tp = __tp_list[i];

			MoveMap* __src_movemap  = NULL;
			MoveMap* __dest_movemap = NULL;;

			if (_movemap_map.find(__map.mapid) == _movemap_map.end())
			{
				fprintf(stdout, "can't find movemap, mapid: %d, coord(%f,%f)\n", __map.mapid, __tp.self_pos.x, __tp.self_pos.z);
				fprintf(stdout, "can't find movemap, dest_mapid: %d, dest_coord(%f,%f)\n", __tp.dest_mapid, __tp.dest_pos.x, __tp.dest_pos.z);
				assert(false);
			}

			if (_movemap_map.find(__tp.dest_mapid) == _movemap_map.end())
			{
				fprintf(stdout, "can't find movemap, mapid: %d, coord(%f,%f)\n", __map.mapid, __tp.self_pos.x, __tp.self_pos.z);
				fprintf(stdout, "can't find dest-movemap, dest_mapid: %d, dest_coord(%f,%f)\n", __tp.dest_mapid, __tp.dest_pos.x, __tp.dest_pos.z);
				assert(false);
			}

			__src_movemap  = _movemap_map[__map.mapid];
			__dest_movemap = _movemap_map[__tp.dest_mapid];

			Node __exit;    //出口(位于本地图)
			Node __entrance;//入口(位于其他地图)

			__exit.mapid     = __map.mapid;
			__entrance.mapid = __tp.dest_mapid;
			__exit.coord     = POINTF_2_POINTI(__src_movemap->TransWorld2Map(__tp.self_pos));
			__entrance.coord = POINTF_2_POINTI(__dest_movemap->TransWorld2Map(__tp.dest_pos));

            if (!__src_movemap->IsPassable(__exit.coord))
            {
                //出口不可达
                fprintf(stderr, "Start point is impassable, self_mapid:%d, start(%f,%f), dest_mapid:%d, end(%f,%f)\n",
                        __exit.mapid, __tp.self_pos.x, __tp.self_pos.z, __tp.dest_mapid, __tp.dest_pos.x, __tp.dest_pos.z);
                assert(false);
            }

            if (!__dest_movemap->IsPassable(__entrance.coord))
            {
                //入口不可达
                fprintf(stderr, "End point is impassable, self_mapid:%d, start(%f,%f), dest_mapid:%d, end(%f,%f)\n",
                        __exit.mapid, __tp.self_pos.x, __tp.self_pos.z, __tp.dest_mapid, __tp.dest_pos.x, __tp.dest_pos.z);
                assert(false);
            }

			_node_list.push_back(__exit);
			_node_list.push_back(__entrance);

			size_t __exit_pos     = _node_list.size() - 2;
			size_t __entrance_pos = _node_list.size() - 1;

			Edge __edge;
			__edge.from  = __exit_pos;
			__edge.to    = __entrance_pos;
			__edge.weight = 1.0f;
			_edge_list.push_back(__edge);

			_mapx_map[__exit.mapid].out_list.push_back(__exit_pos);
			_mapx_map[__entrance.mapid].in_list.push_back(__entrance_pos);
		}
	}
}

void GlobalTransDataGenerator::GenerateAllEdges()
{
	//产生本地图内所有的EDGE
	//每个入口到传送点均产生一条路径

	std::map<MapID, MapX>::iterator it = _mapx_map.begin();
	for (; it != _mapx_map.end(); ++ it)
	{
		for (size_t i = 0; i < it->second.in_list.size(); ++ i)
		{
			int32_t from = it->second.in_list[i];
			for (size_t j = 0; j < it->second.out_list.size(); ++ j)
			{
				int32_t to = it->second.out_list[j];
				Node& node_from = _node_list[from];
				Node& node_to   = _node_list[to];

				assert(node_from.mapid == node_to.mapid);
				assert(_movemap_map.find(node_from.mapid) != _movemap_map.end());
				MoveMap* __movemap = _movemap_map[node_from.mapid];

				const Cluster* pSrcCluster = __movemap->LocateCluster(node_from.coord);
				const Cluster* pDstCluster = __movemap->LocateCluster(node_to.coord);
				assert(pSrcCluster && pDstCluster);
				
				//位于同一簇
				if (pSrcCluster == pDstCluster)
				{
					Path path;
					MAStar mAstar;
					if (mAstar.Search(__movemap->GetBitMap(), node_from.coord, node_to.coord, path, pSrcCluster))
					{
						Edge __edge;
						__edge.from   = from;
						__edge.to     = to;
						__edge.weight = Distance(path);
						_edge_list.push_back(__edge);
					}
					else
					{
                        PRINT_NODE_PAIR(node_from, node_to);
						assert(false);
					}
					continue;
				}

				//位于不同簇
				//先搜索在本地图内的路由
				Route route;
				PfEngine engine(__movemap);
				engine.Setup(node_from.coord, node_to.coord);
				int state = engine.Search(route);
				if (state != PF_ENGINE_STATE_FOUND)
				{
                    PRINT_NODE_PAIR(node_from, node_to);
					assert(false);
					engine.Cleanup();
					return;
				}
				engine.Cleanup();

				if (route.size() < 4)
				{
                    PRINT_NODE_PAIR(node_from, node_to);
					assert(false);
				}
				assert(route.size() % 2 == 0);

				/**
				 * 根据路由在各自的区域搜索
				 */
				float __distance = 0.0f;
				for (size_t i = 0; i < route.size(); i += 2)
				{
					const VexNode* __node_from = route[i];
					const VexNode* __node_to = route[i+1];
					assert(__node_from->GetOwner() == __node_to->GetOwner());

					PointI __start = __node_from->GetCoord();
					PointI __goal  = __node_to->GetCoord();
					const Cluster* __cluster = __movemap->LocateCluster(__start);
					assert(__cluster);

					Path path;
					MAStar mAstar;
					if (mAstar.Search(__movemap->GetBitMap(), __start, __goal, path, __cluster))
					{
						__distance += Distance(path);
					}
					else
					{
						assert(false);
						return;
					}
				}

				Edge __edge;
				__edge.from   = from;
				__edge.to     = to;
				__edge.weight = __distance;
				_edge_list.push_back(__edge);

				//这2个节点的内存在调用RmvVetex的时候没有被真正释放，因为上面的for循环使用这2个节点
				//将来考虑Route里面使用非指针来解决这个异步释放内存问题
				VexNode* __start_node = const_cast<VexNode*>(route.front());
				VexNode* __goal_node  = const_cast<VexNode*>(route.back());
				delete __start_node;
				delete __goal_node;
			}
		}
	}
}

void GlobalTransDataGenerator::GenerateTransTable()
{
	TransTable* __trans_table = new TransTable();
	if (!__trans_table)
		return;

	std::map<MapID, MapX>::iterator it = _mapx_map.begin();
	for (; it != _mapx_map.end(); ++ it)
	{
		MapX& __src_mapx = it->second;

		TransTable::MapEntry& __dest_map = __trans_table->_map[it->first];

		for (size_t i = 0; i < __src_mapx.in_list.size(); ++ i)
			__dest_map.in_list.push_back(__src_mapx.in_list[i]);

		for (size_t i = 0; i < __src_mapx.out_list.size(); ++ i)
			__dest_map.out_list.push_back(__src_mapx.out_list[i]);
	}

	_trans_table = __trans_table;
}

void GlobalTransDataGenerator::GenerateTransGraph()
{
	DGraph* dgraph = new DGraph();
	if (!dgraph)
		return;

	for (size_t i = 0; i < _node_list.size(); ++ i)
	{
		Node& node  = _node_list[i];
		VexNode* __node = new VexNode(node.mapid, node.coord.x, node.coord.z);
		if (!__node)
			return;

		dgraph->AddVetex(__node);
	}

	for (size_t i = 0; i < _edge_list.size(); ++ i)
	{
		Edge& edge = _edge_list[i];
		ArcNode* __edge = new ArcNode(edge.weight, edge.from, edge.to);
		if (!__edge)
			return;

		dgraph->AddArc(__edge);
	}

	_trans_graph = dgraph;
}

void GlobalTransDataGenerator::dump()
{
	fprintf(stdout, "_node_list(%ld)\n", _node_list.size());
	for (size_t i = 0; i < _node_list.size(); ++ i)
	{
		Node& __node = _node_list[i];
		fprintf(stdout, "node%ld(mapid=%d,pos(%d,%d))\n", i, __node.mapid, __node.coord.x, __node.coord.z);
	}

	fprintf(stdout, "_edge_list(%ld)\n", _edge_list.size());
	for (size_t i = 0; i < _edge_list.size(); ++ i)
	{
		Edge& __edge = _edge_list[i];
		fprintf(stdout, "edge%ld(from=%d(%d,%d),to=%d(%d,%d),weight=%f),", i, __edge.from, _node_list[__edge.from].coord.x, _node_list[__edge.from].coord.z, __edge.to, _node_list[__edge.to].coord.x, _node_list[__edge.to].coord.z, __edge.weight);
		fprintf(stdout, ")\n");
	}

	fprintf(stdout, "_mapx_map(%ld)\n", _mapx_map.size());
	std::map<MapID, MapX>::iterator it = _mapx_map.begin();
	for (; it != _mapx_map.end(); ++ it)
	{
		MapX& __mapx = it->second;
		fprintf(stdout, "mapid(%d), in_list(", it->first);
		for (size_t j = 0; j < __mapx.in_list.size(); ++ j)
			fprintf(stdout, "%d,", __mapx.in_list[j]);
		fprintf(stdout, "), ");
		fprintf(stdout, "out_list(");
		for (size_t j = 0; j < __mapx.out_list.size(); ++ j)
			fprintf(stdout, "%d,", __mapx.out_list[j]);
		fprintf(stdout, "))\n");
	}
}

};
