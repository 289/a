#ifndef GAMED_GS_OBJ_SERVICE_MAN_H_
#define GAMED_GS_OBJ_SERVICE_MAN_H_

#include <map>
#include <set>

#include "shared/base/singleton.h"


namespace gamed {

class ServiceInserter;
class ServiceProvider;
class ServiceExecutor;

///
/// ServiceManager
///
class ServiceManager : public shared::Singleton<ServiceManager>
{
	friend class shared::Singleton<ServiceManager>;
	friend class ServiceInserter;

	typedef std::map<int, ServiceProvider*> ProviderMap;
	typedef std::map<int, ServiceExecutor*> ExecutorMap;
	typedef std::set<int> UISupportSet;
public:
	static inline ServiceManager* GetInstance() {
		return &(get_mutable_instance());
	}

	static ServiceProvider* CreateProvider(int type);
	static const ServiceProvider* GetProvider(int type);
	static const ServiceExecutor* GetExecutor(int type);
	static bool  IsUISupportService(int type);

protected:
	ServiceManager();
	~ServiceManager();

private:
	bool InsertService(ServiceProvider* provider, ServiceExecutor* executor, bool is_ui_support);
	ServiceProvider* getProvider(int type);
	ServiceExecutor* getExecutor(int type);
	bool IsUISupport(int type) const;

private:
	ProviderMap  provider_map_;
	ExecutorMap  executor_map_;
	UISupportSet ui_support_set_;  // UI可用的服务
};

#define s_pServiceMan gamed::ServiceManager::GetInstance()


///
/// ProviderList
///
class ProviderList
{
	typedef std::map<int, ServiceProvider*> LIST;
public:
	ProviderList(size_t capacity);
	~ProviderList();

	bool AddProvider(ServiceProvider* provider);
	ServiceProvider* GetProvider(int type);

protected:
	void Clear();

private:
	LIST   list_;
	size_t capacity_;
};

///
/// ServiceProvider
///
class ServiceInserter
{
public:
	template <typename PROVIDER, typename EXECUTOR>
	ServiceInserter(PROVIDER*, EXECUTOR*, bool is_ui_support)
	{
		s_pServiceMan->InsertService(new PROVIDER, new EXECUTOR, is_ui_support);
	}
};

#define SERVICE_INSERTER(PROVIDER, EXECUTOR, IS_UI_SUPPORT) \
	ServiceInserter((PROVIDER*)NULL, (EXECUTOR*)NULL, IS_UI_SUPPORT);

#define INIT_SERVICE_PROTOTYPE(PROVIDER, EXECUTOR, IS_UI_SUPPORT) \
	static ServiceInserter PROVIDER##EXECUTOR = SERVICE_INSERTER(PROVIDER, EXECUTOR, IS_UI_SUPPORT);

} // namespace gamed

#endif // GAMED_GS_OBJ_SERVICE_MAN_H_
