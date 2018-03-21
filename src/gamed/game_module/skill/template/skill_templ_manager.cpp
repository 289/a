#include <stdio.h>
#include <stdlib.h>
#include "shared/net/packet/codec_templ.h"
#include "skill_templ_manager.h"
#include "skill_templ.h"
#include "effect_templ.h"

namespace skill
{

using namespace shared;
using namespace shared::net;
using namespace std;

#define SKILLTEMPLATE_VERSION 0x00000001

DataTemplManager::DataTemplManager()
	: codec_(NULL)
{
	codec_ = new TemplPacketCodec<BaseTempl>(SKILLTEMPLATE_VERSION,
											 BaseTemplCreater::CreatePacket,
											 BaseTemplCreater::IsValidType);
	assert(codec_ != NULL);
}

DataTemplManager::~DataTemplManager()
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

bool DataTemplManager::ReadFromFile(const char* file)
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

bool DataTemplManager::ReadFromBuffer(const Buffer& buffer)
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
		templ_map_[templ_type][templ->templ_id] = templ;
	}
	tempvec.clear();

	LoadComplete();
	return true;
}

bool DataTemplManager::WriteToFile(const char* file, vector<BaseTempl*>& vec_templ)
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

void DataTemplManager::LoadComplete()
{
	IdToTemplMap& id_query_map = templ_map_[TEMPL_TYPE_SKILL];
	IdToTemplMap::iterator it = id_query_map.begin();
	for (; it != id_query_map.end(); ++it)
	{
		SkillTempl* skill = static_cast<SkillTempl*>(it->second);
		AttackFrameVec::iterator ait = skill->attack_frame.begin();
		for (; ait != skill->attack_frame.end(); ++ait)
		{
			for (int32_t i = 0; i <= 1; ++i)
			{
				FrameVec& frame_vec = i == 0 ? ait->normal : ait->redir;
				EffectTemplVec& templ_vec = i == 0 ? ait->effect_templ : ait->redir_templ;
				FrameVec::const_iterator fit = frame_vec.begin();
				for (; fit != frame_vec.end(); ++fit)
				{
					templ_vec.push_back(GetEffectTempl(fit->effect_id));
				}
			}
		}
	}
}

} // namespace skill 
