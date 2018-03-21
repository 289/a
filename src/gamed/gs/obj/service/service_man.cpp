#include "service_man.h"

#include "service_provider.h"
#include "service_executor.h"


namespace gamed {

///
/// 初始化服务类型的原型模型
///

// ---- service: 0
/* serviceDef::SRV_DEF_INVALID*/
INIT_SERVICE_PROTOTYPE(SimpleServiceProvider, SimpleServiceExecutor, true); // 1
INIT_SERVICE_PROTOTYPE(DeliverTaskProvider, DeliverTaskExecutor, false);
INIT_SERVICE_PROTOTYPE(RecycleTaskProvider, RecycleTaskExecutor, false);
INIT_SERVICE_PROTOTYPE(ShopProvider, ShopExecutor, false);

// ---- service: 5
INIT_SERVICE_PROTOTYPE(StorageTaskProvider, StorageTaskExecutor, false);
INIT_SERVICE_PROTOTYPE(EnhanceProvider, EnhanceExecutor, false);



///
/// ServiceManager
///
// ---- static member function ----
ServiceProvider* ServiceManager::CreateProvider(int type)
{
	ServiceProvider* provider = s_pServiceMan->getProvider(type);
	if (provider == NULL)
	{
		ASSERT(false && "can not find provider");
		return NULL;
	}
	return provider->Clone();
}

const ServiceProvider* ServiceManager::GetProvider(int type)
{
	return s_pServiceMan->getProvider(type);
}

const ServiceExecutor* ServiceManager::GetExecutor(int type)
{
	return s_pServiceMan->getExecutor(type);
}

bool ServiceManager::IsUISupportService(int type)
{
	return s_pServiceMan->IsUISupport(type);
}

// ---- member function ----
ServiceManager::ServiceManager()
{
}

ServiceManager::~ServiceManager()
{
	ProviderMap::iterator it_provider = provider_map_.begin();
	for (; it_provider != provider_map_.end(); ++it_provider)
	{
		DELETE_SET_NULL(it_provider->second);
	}
	provider_map_.clear();

	ExecutorMap::iterator it_executor = executor_map_.begin();
	for (; it_executor != executor_map_.end(); ++it_executor)
	{
		DELETE_SET_NULL(it_executor->second);
	}
	executor_map_.clear();
}

bool ServiceManager::InsertService(ServiceProvider* provider, ServiceExecutor* executor, bool is_ui_support)
{
	// provider的type可能和executor不一样，一些provider只做简单的feedback所以写成一个类
	ASSERT(executor->GetType() > serviceDef::SRV_DEF_INVALID && executor->GetType() < serviceDef::SRV_DEF_MAX);
	ASSERT(provider->GetType() == executor->GetType() || provider->GetType() == -100);

	int type   = executor->GetType();
	bool bRst1 = provider_map_.insert(std::pair<int, ServiceProvider*>(type, provider)).second;
	bool bRst2 = executor_map_.insert(std::pair<int, ServiceExecutor*>(type, executor)).second;
	ASSERT(bRst1 && bRst2);
	if (!bRst1 || !bRst2)
	{
		return false;
	}

	bool bRst3 = ui_support_set_.insert(type).second;
	ASSERT(bRst3);
	if (!bRst3) 
	{
		return false;
	}
	return true;
}

ServiceProvider* ServiceManager::getProvider(int type)
{
	ProviderMap::iterator it = provider_map_.find(type);
	if (it != provider_map_.end())
	{
		return it->second;
	}
	return NULL;
}

ServiceExecutor* ServiceManager::getExecutor(int type)
{
	ExecutorMap::iterator it = executor_map_.find(type);
	if (it != executor_map_.end())
	{
		return it->second;
	}
	return NULL;
}

bool ServiceManager::IsUISupport(int type) const
{
	UISupportSet::iterator it = ui_support_set_.find(type);
	if (it != ui_support_set_.end())
	{
		return true;
	}
	return false;
}


///
/// ProviderList
///
ProviderList::ProviderList(size_t capacity)
	: capacity_(capacity)
{
}
	
ProviderList::~ProviderList()
{
	Clear();
}

bool ProviderList::AddProvider(ServiceProvider* provider)
{
	if (list_.size() >= capacity_)
		return false;

	if (!(list_.insert(std::pair<int, ServiceProvider*>(provider->GetType(), provider)).second))
	{
		return false;
	}
	return true;
}
	
void ProviderList::Clear()
{
	LIST::iterator it = list_.begin();
	for (; it != list_.end(); ++it)
	{
		DELETE_SET_NULL(it->second);
	}
	list_.clear();
}

ServiceProvider* ProviderList::GetProvider(int type)
{
	LIST::iterator it = list_.find(type);
	if (it == list_.end())
	{
		return NULL;
	}
	return it->second;
}

} // namespace gamed
