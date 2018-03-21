#include "extratempl_man.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared/net/packet/codec_templ.h"


namespace extraTempl {

static std::string kFileSuffixName = ".gbd";  // game binary data

ExtraTemplManager::ExtraTemplManager()
	: codec_(NULL)
{
	codec_ = new TemplPacketCodec<BaseExtraTempl>(EXTRA_TEMPLATE_VERSION, 
			                                      BaseExtraTemplManager::CreatePacket, 
												  BaseExtraTemplManager::IsValidType);
	assert(codec_);
}

ExtraTemplManager::~ExtraTemplManager()
{
	TypeQueryMap::const_iterator it = type_query_map_.begin();
	for (; it != type_query_map_.end(); ++it)
	{
		const IdToTemplMap& baseptr_map = it->second;
		IdToTemplMap::const_iterator it_tpl = baseptr_map.begin();
		for (; it_tpl != baseptr_map.end(); ++it_tpl)
		{
			delete it_tpl->second;
		}
	}
	type_query_map_.clear();

	DELETE_SET_NULL(codec_);
}

bool ExtraTemplManager::ReadFromFile(const char* file)
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
	if (!ReadFromBuffer(buffer))
	{
		fclose(pfile);
		return false;
	}

	// check monster-group
	if (!CheckTemplateAfterRWFile(type_query_map_))
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

bool ExtraTemplManager::ReadFromBuffer(const shared::net::Buffer& buffer)
{
	// parse
	std::vector<BaseExtraTempl*> tempvec;
	if (TPC_SUCCESS != codec_->ParseBuffer(&buffer, tempvec))
	{
		return false;
	}

	// push to map
	// 每一个文件只能是一种类型的extra_templ
	const int packet_type = tempvec[0]->GetType();
	for (size_t i = 0; i < tempvec.size(); ++i)
	{
		assert(packet_type == tempvec[i]->GetType());
		assert(tempvec[i]->CheckDataValidity());
		InsertToSpecificMap(tempvec[i]->templ_id, tempvec[i]);
	}
	tempvec.clear();

	return true;
}
	
bool ExtraTemplManager::WriteToFile(const char* path, std::vector<BaseExtraTempl*>& vec_templ)
{
	typedef std::vector<BaseExtraTempl*> BaseTemplVec;
	typedef std::map<PacketType, BaseTemplVec> ClassifyMap;
	ClassifyMap tmp_classify_map;
	for (size_t i = 0; i < vec_templ.size(); ++i)
	{
		if (!vec_templ[i]->CheckDataValidity()) 
		{
			return false;
		}
		tmp_classify_map[vec_templ[i]->GetType()].push_back(vec_templ[i]);	
	}

	ClassifyMap::iterator it_type = tmp_classify_map.begin();
	for (; it_type != tmp_classify_map.end(); ++it_type)
	{
		BaseTemplVec& tmp_templ_vec = it_type->second;
		if (tmp_templ_vec.size() == 0)
			return false;

		std::string file = path;
		file = file + tmp_templ_vec[0]->TemplateName() + kFileSuffixName;

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

	TypeQueryMap type_map;
	CopyToTypeMap(vec_templ, type_map);
	if (!CheckTemplateAfterRWFile(type_map))
	{
		fprintf(stderr, "Check error after write!");
		return false;
	}

	return true;
}

void ExtraTemplManager::InsertToSpecificMap(TemplID templ_id, const BaseExtraTempl* ptempl)
{
    std::pair<IdToTemplMap::iterator, bool> ret;
    ret = type_query_map_[ptempl->GetType()].insert(std::pair<TemplID, const BaseExtraTempl*>(templ_id, ptempl));
    assert(ret.second); (void)ret;
}

void ExtraTemplManager::CopyToTypeMap(const std::vector<BaseExtraTempl*>& vec_templ, TypeQueryMap& type_map)
{
	for (size_t i = 0; i < vec_templ.size(); ++i)
	{
		TemplID templ_id = vec_templ[i]->templ_id;
		const BaseExtraTempl* ptempl = vec_templ[i];
        std::pair<IdToTemplMap::iterator, bool> ret;
		ret = type_map[ptempl->GetType()].insert(std::pair<TemplID, const BaseExtraTempl*>(templ_id, ptempl));
        assert(ret.second); (void)ret;
	}
}

bool ExtraTemplManager::CheckTemplateAfterRWFile(const TypeQueryMap& type_map)
{
	TypeQueryMap::const_iterator it = type_map.find(TEMPL_TYPE_MONSTER_GROUP_TEMPL);
	if (it != type_map.end())
	{
		if (!CheckMonsterGroupData(it->second))
			return false;
	}

	return true;
}

bool ExtraTemplManager::CheckMonsterGroupData(const IdToTemplMap& id_map) const
{
	/**
	 * 怪物组中可能配置了下一波怪物的怪物组ID
	 * 这种配置方式可能造成循环，这里做了检查
	 */
	const IdToTemplMap& __map = id_map;
	for (IdToTemplMap::const_iterator it = __map.begin(); it != __map.end(); ++ it)
	{
		const MonsterGroupTempl* tpl = dynamic_cast<const MonsterGroupTempl*>(it->second);

		std::set<TemplID> ids;
		ids.insert(tpl->templ_id);

		while (tpl->next_monster_group > 0)
		{
			if (!ids.insert(tpl->next_monster_group).second)
			{
				fprintf(stderr, "monster group circulation error! tid:%d \n", tpl->templ_id);
				return false;
			}
			if (ids.size() >= 10)
			{
				fprintf(stderr, "monster group circulation is greater than 10 \n");
				return false;
			}
			tpl = dynamic_cast<const MonsterGroupTempl*>(__map.find(tpl->next_monster_group)->second);
		}
	}

	return true;
}

} // namespace gamed
