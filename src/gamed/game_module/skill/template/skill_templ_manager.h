#ifndef SKILL_DATATEMPL_TEMPL_MANAGER_H_
#define SKILL_DATATEMPL_TEMPL_MANAGER_H_

#include "base_skill_templ.h"

namespace shared
{
namespace net
{
class Buffer;
template<typename T>
class TemplPacketCodec;
} // namespace net
} // namespace shared

namespace skill
{

class SkillTempl;
class EffectTempl;

// 数据模版容器
class DataTemplManager : public shared::Singleton<DataTemplManager>
{
	friend class shared::Singleton<DataTemplManager>;
public:
	static inline DataTemplManager* GetInstance() 
	{
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool ReadFromBuffer(const shared::net::Buffer& buffer);
	bool WriteToFile(const char* file, std::vector<BaseTempl*>& vec_templ);

	template<class T> const T* QueryDataTempl(TemplType type, TemplID id);
protected:
	DataTemplManager();
	~DataTemplManager();

	void LoadComplete();
private:
	shared::net::TemplPacketCodec<BaseTempl>* codec_;
	typedef std::map<TemplID, BaseTempl*> IdToTemplMap;
	typedef std::map<TemplType, IdToTemplMap> DataTemplMap;
	DataTemplMap templ_map_;
};

template<class T>
const T* DataTemplManager::QueryDataTempl(TemplType type, TemplID id)
{
	DataTemplMap::const_iterator mit = templ_map_.find(type);
	assert(mit != templ_map_.end());
	const IdToTemplMap& id_query_map = mit->second;
	IdToTemplMap::const_iterator it = id_query_map.find(id);
	assert(it != id_query_map.end());
	const BaseTempl* templ = it->second;
	assert(templ != NULL && templ->GetType() == type && templ->templ_id == id);
	return dynamic_cast<const T*>(templ);
}

#define s_pSkillTempl skill::DataTemplManager::GetInstance()
#define GetSkillTempl(id) \
	s_pSkillTempl->QueryDataTempl<SkillTempl>(TEMPL_TYPE_SKILL, id)
#define GetEffectTempl(id) \
	s_pSkillTempl->QueryDataTempl<EffectTempl>(TEMPL_TYPE_EFFECT, id)

} // namespace skill

#endif // SKILL_DATATEMPL_TEMPL_MANAGER_H_
