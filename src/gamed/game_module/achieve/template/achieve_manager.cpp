#include <stdio.h>
#include <stdlib.h>
#include "shared/net/packet/codec_templ.h"
#include "achieve_manager.h"
#include "achieve_templ.h"

namespace achieve
{

using namespace shared;
using namespace shared::net;
using namespace std;

#define ACHIEVETEMPLATE_VERSION 0x00000001

AchieveManager::AchieveManager()
	: codec_(NULL)
{
	codec_ = new TemplPacketCodec<BaseTempl>(ACHIEVETEMPLATE_VERSION,
											 BaseTemplCreater::CreatePacket,
											 BaseTemplCreater::IsValidType);
	assert(codec_ != NULL);
}

AchieveManager::~AchieveManager()
{
	DataTemplMap::const_iterator mit = templ_map_.begin();
	for (; mit != templ_map_.end(); ++mit)
	{
		const IdToTemplMap& id_query_map = mit->second;
		IdToTemplMap::const_iterator it = id_query_map.begin();
		for (; it != id_query_map.end(); ++it)
		{
			if (it->second != NULL)
			{
				delete it->second;
			}
		}
	}
	templ_map_.clear();

	delete codec_;
	codec_ = NULL;
}

bool AchieveManager::ReadFromFile(const char* file)
{
	FILE* pfile       = NULL;
	int32_t file_size = 0;
	int result        = 0;

	if ((pfile = fopen(file, "rb")) == NULL)
		return false;

	// 获取文件大小
	fseek(pfile, 0L, SEEK_END);
	file_size = ftell(pfile);
	rewind(pfile);

	// 分配Buffer
	Buffer buffer;
	buffer.EnsureWritableBytes(file_size);

	// 拷贝整个文件
	result = fread(buffer.BeginWrite(), 1, file_size, pfile);
	if (result != file_size)
	{
		fclose(pfile);
		return false;
	}
	buffer.HasWritten(result);
	
	// 解析
	if (!ReadFromBuffer(buffer))
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

bool AchieveManager::ReadFromBuffer(const Buffer& buffer)
{
	vector<BaseTempl*> tempvec;
	if (TPC_SUCCESS != codec_->ParseBuffer(&buffer, tempvec))
	{
		return false;
	}

	for (size_t i = 0; i < tempvec.size(); ++i)
	{
		BaseTempl* templ = tempvec[i];
		assert(templ->CheckDataValidity());
		TemplType templ_type = templ->GetType();
		templ_map_[templ_type][templ->id] = templ;
	}
	tempvec.clear();

	LoadComplete();
	return true;
}

bool AchieveManager::WriteToFile(const char* file, vector<BaseTempl*>& vec_templ)
{
	for (size_t i = 0; i < vec_templ.size(); ++i)
	{
		if (!vec_templ[i]->CheckDataValidity()) 
		{
			return false;
		}
	}

	Buffer buffer;
	if (TPC_SUCCESS != codec_->AssemblingEmptyBuffer(&buffer, vec_templ))
	{
		return false;
	}
	
	FILE* pfile = NULL;
	if ((pfile = fopen(file, "wb+")) == NULL)
		return false;

	size_t result = fwrite(buffer.peek(), 1, buffer.ReadableBytes(), pfile);
	if (result != buffer.ReadableBytes())
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

const AchieveTempl* AchieveManager::GetAchieve(AchieveID id) const
{
	return QueryDataTempl<AchieveTempl>(TEMPL_TYPE_ACHIEVE, id);
}

const IdToTemplMap* AchieveManager::GetTemplMap(TemplType type) const
{
    DataTemplMap::const_iterator it = templ_map_.find(type);
    return it == templ_map_.end() ? NULL : &(it->second);
}

void AchieveManager::LoadComplete()
{
	UpdateAchieveTempl();
}

void AchieveManager::UpdateAchieveTempl()
{
	IdToTemplMap& id_query_map = templ_map_[TEMPL_TYPE_ACHIEVE];
	IdToTemplMap::iterator it = id_query_map.begin();
	for (; it != id_query_map.end(); ++it)
	{
		AchieveTempl* achieve = static_cast<AchieveTempl*>(it->second);
		// 初始化树节点指针
		achieve->parent_templ = GetAchieve(achieve->parent);
		achieve->next_sibling_templ = GetAchieve(achieve->next_sibling);
		achieve->first_child_templ = GetAchieve(achieve->first_child);
	}
}

} // namespace achieve
