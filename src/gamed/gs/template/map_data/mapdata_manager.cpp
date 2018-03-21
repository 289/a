#include "mapdata_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits> // std::numeric_limits

#include "shared/net/packet/codec_templ.h"


namespace mapDataSvr {

static const char* s_filename_prefix = "mapdata_";
static const char* s_filename_suffix = ".gbd";
static const char* s_path_separator  = "/";

namespace {

	static const std::string itos(int64_t n)
	{
		const int max_size = std::numeric_limits<int64_t>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
		char buffer[max_size] = {0};
#ifdef PLATFORM_WINDOWS
		_snprintf(buffer, max_size, "%lld", n);
#else // !PLATFORM_WINDOWS
		snprintf(buffer, max_size, "%ld", n);
#endif // PLATFORM_WINDOWS
		return std::string(buffer);
	}

	static const std::string make_name(const char* path, MapID mapid)
	{
		std::string tmpname = path;
		tmpname += s_path_separator;
		tmpname += s_filename_prefix;
		tmpname += itos(mapid);
		tmpname += s_filename_suffix;
		return tmpname;
	}

} // Anonymous

MapDataManager::MapDataManager()
	: codec_(NULL)
{
	codec_ = new TemplPacketCodec<BaseMapData>(MAPDATA_VERSION, 
			                                   BaseMapDataManager::CreatePacket, 
											   BaseMapDataManager::IsValidType);
	assert(codec_);
}

MapDataManager::~MapDataManager()
{
	IdToMapDataMap::const_iterator it = id_query_map_.begin();
	for (; it != id_query_map_.end(); ++it)
	{
		delete it->second;
	}
	id_query_map_.clear();

	DELETE_SET_NULL(codec_);
}

bool MapDataManager::ReadFromFile(const char* file)
{
	FILE* pfile       = NULL;
	int32_t file_size = 0;
	int result        = 0;

	if ((pfile = fopen(file, "rb")) == NULL)
		return false;

	// get file size
	fseek(pfile, 0L, SEEK_END);
	file_size = ftell(pfile);
	rewind(pfile);

	// malloc buffer
	shared::net::Buffer buffer;
	buffer.EnsureWritableBytes(file_size);

	// copy whole file
	result = fread(buffer.BeginWrite(), 1, file_size, pfile);
	if (result != file_size)
	{
		fclose(pfile);
		return false;
	}
	buffer.HasWritten(result);

	// parse
	std::vector<BaseMapData*> tempvec;
	if (TPC_SUCCESS != codec_->ParseBuffer(&buffer, tempvec))
	{
		fclose(pfile);
		return false;
	}

	// push to map
	for (size_t i = 0; i < tempvec.size(); ++i)
	{
		BaseMapData* pelem = tempvec[i];
		assert(pelem->CheckDataValidity());
		assert(id_query_map_.insert(std::pair<ElemID, const BaseMapData*>(pelem->elem_id, pelem)).second);
		InsertToMapByMapID(pelem);
	}
	tempvec.clear();

	fclose(pfile);
	return true;
}

	
bool MapDataManager::WriteToFile(const char* path, std::vector<BaseMapData*>& vec_mapdata)
{
	typedef std::vector<BaseMapData*> BaseTemplVec;
	typedef std::map<MapID, BaseTemplVec> ClassifyMap;
	ClassifyMap tmp_classify_map;
	for (size_t i = 0; i < vec_mapdata.size(); ++i)
	{
		if (!vec_mapdata[i]->CheckDataValidity()) 
		{
			return false;
		}
		tmp_classify_map[vec_mapdata[i]->map_id].push_back(vec_mapdata[i]);
	}

	ClassifyMap::iterator it_type = tmp_classify_map.begin();
	for (; it_type != tmp_classify_map.end(); ++it_type)
	{
		BaseTemplVec& tmp_templ_vec = it_type->second;
		if (tmp_templ_vec.size() == 0)
			return false;

		std::string file;
		file = make_name(path, tmp_templ_vec[0]->map_id);

		shared::net::Buffer buffer;
		if (TPC_SUCCESS != codec_->AssemblingEmptyBuffer(&buffer, tmp_templ_vec))
		{
			return false;
		}

		FILE* pfile = NULL;
		if ((pfile = fopen(file.c_str(), "wb+")) == NULL)
			return false;

		size_t result = fwrite(buffer.peek(), 1, buffer.ReadableBytes(), pfile);
		if (result != buffer.ReadableBytes())
		{
			fclose(pfile);
			return false;
		}

		fclose(pfile);
	}

	return true;
}

void MapDataManager::InsertToMapByMapID(BaseMapData* pelem)
{
	mapid_query_map_[pelem->map_id][pelem->GetType()].push_back(pelem);
	if (pelem->GetType() == MAPDATA_TYPE_MAP_DEFAULT_AREA)
	{
		assert(mapid_query_map_[pelem->map_id][pelem->GetType()].size() == 1);
	}
}

const BaseMapData* MapDataManager::QueryBaseMapDataTempl(ElemID id)
{
	IdToMapDataMap::const_iterator it = id_query_map_.find(id);
	if (it == id_query_map_.end())
		return NULL;
	return it->second;
}

} // namespace gamed
