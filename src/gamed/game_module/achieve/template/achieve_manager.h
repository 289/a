#ifndef ACHIEVE_ACHIEVE_MANAGER_H_
#define ACHIEVE_ACHIEVE_MANAGER_H_

#include "base_achieve_templ.h"

namespace shared
{
namespace net
{
class Buffer;
template<typename T>
class TemplPacketCodec;
} // namespace net
} // namespace shared

namespace achieve
{

class AchieveTempl;
typedef std::map<TemplID, BaseTempl*> IdToTemplMap;
typedef std::map<TemplType, IdToTemplMap> DataTemplMap;

// 成就数据管理器
class AchieveManager : public shared::Singleton<AchieveManager>
{
	friend class shared::Singleton<AchieveManager>;
public:
	static inline AchieveManager* GetInstance() 
	{
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool ReadFromBuffer(const shared::net::Buffer& buffer);
	bool WriteToFile(const char* file, std::vector<BaseTempl*>& vec_templ);

	template<class T> const T* QueryDataTempl(TemplType type, TemplID id) const;
    const IdToTemplMap* GetTemplMap(TemplType type) const;

	const AchieveTempl* GetAchieve(AchieveID id) const;
protected:
	AchieveManager();
	~AchieveManager();

	void LoadComplete();
	void UpdateAchieveTempl();
private:
	shared::net::TemplPacketCodec<BaseTempl>* codec_;
	DataTemplMap templ_map_;
};

template<class T>
const T* AchieveManager::QueryDataTempl(TemplType type, TemplID id) const
{
	DataTemplMap::const_iterator mit = templ_map_.find(type);
	assert(mit != templ_map_.end());
	const IdToTemplMap& id_query_map = mit->second;
	IdToTemplMap::const_iterator it = id_query_map.find(id);
	if (it == id_query_map.end())
	{
		return NULL;
	}
	const BaseTempl* templ = it->second;
	assert(templ != NULL && templ->GetType() == type && templ->id == id);
	return dynamic_cast<const T*>(templ);
}

#define s_pAchieve AchieveManager::GetInstance()

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_MANAGER_H_
