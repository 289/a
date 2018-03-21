#ifndef GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_DEF_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_DEF_H_

#include "templ_types.h"
#include "base_datatempl.h"
#include "shared/base/singleton.h"
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bit_set.h"


namespace serviceDef {

typedef int32_t TaskID;

using namespace shared;
using namespace shared::net;

/**
 * @brief 服务类型号的定义，客户端根据服务号来请求服务
 *    （1）以下type在游戏上线后不能修改顺序，只能在最后添加，因为会存盘
 */
enum
{
  // 0
	SRV_DEF_INVALID = 0,
	SRV_DEF_SIMPLE_SERVICE,
	SRV_DEF_DELIVER_TASK,
	SRV_DEF_RECYCLE_TASK,
	SRV_DEF_SHOP,

  // 5
	SRV_DEF_STORAGE_TASK,
    SRV_DEF_ENHANCE,

	SRV_DEF_MAX
};

/**
 * @brief 简单服务
 *    （1）不需要带参数或者参数是固定的服务
 *    （2）以下type在游戏上线后不能修改顺序，只能在最后添加，因为会存盘
 *    （3）以下服务注释标明为"必须"项时，编辑器不需要显示
 */
enum SimpleServiceType
{
	SS_TRANSFER_TEAM_MEMBER = 0, // player身上的服务，该项是"必须"的
};

class ServiceTempl;


///
/// ---- 功能函数 ----
///
/**
 * @brief FillEmptyBuffer 
 *    （1）FillEmptyBuffer会调用每个ServiceTempl的CheckDataValidity()来
 *         检查ServiceTempl是否有效，调用者需要检查返回值判断是否成功
 */
bool FillEmptyBuffer(std::string& buf, const std::vector<ServiceTempl*>& vecTemplPtr);
/**
 * @brief ParseBuffer 
 *    （1）如果ParseBuffer返回false，调用者需要把vecTemplPtr里的指针全部delete掉
 *    （2）vecTemplPtr里的ServiceTempl*指针是new出来的，TODO:返回false则需要调用者来delete vecTemplPtr里的指针
 */
bool ParseBuffer(const std::string& buf, std::vector<ServiceTempl*>& vecTemplPtr);


/**
 * @brief ServiceTempl
 *    （1）服务模板是比较特殊的，它并不是一个BaseDataTempl，而是作为流的形式保存在
 *         service_npc_templ的一个成员变量里。这样做是为了能方便的读取各种服务类型
 *         的变长初始化数据。
 *    （2）新定义的非简单服务需要在cpp里调用INIT_STAITC_SERVICE_TEMPL进行初始化
 */
class ServiceTempl : public shared::net::BasePacket
{
public:
	typedef shared::net::BasePacket::Type SrvType;
	
	ServiceTempl(SrvType type)
		: shared::net::BasePacket(type)
	{ }
	virtual ~ServiceTempl() { }
	bool CheckDataValidity() const { return OnCheckDataValidity(); }

protected:
	virtual bool OnCheckDataValidity() const = 0;
};


///
/// ServiceTemplManager
///
class ServiceTemplManager : public shared::Singleton<ServiceTemplManager>
{
	friend class shared::Singleton<ServiceTemplManager>;
public:
	static inline ServiceTemplManager* GetInstance() {                 
		return &(get_mutable_instance());
	}
	static bool InsertPacket(uint16_t type, ServiceTempl* packet);
	static ServiceTempl* CreatePacket(ServiceTempl::SrvType id);
	static bool IsValidType(int32_t type);

	bool OnInsertPacket(uint16_t type, ServiceTempl* packet);
	ServiceTempl* OnCreatePacket(ServiceTempl::SrvType id);

private:
	typedef std::map<ServiceTempl::SrvType, ServiceTempl*> ServiceTemplMap;
	ServiceTemplMap packet_map_;
};

#define INIT_STAITC_SERVICE_TEMPL(class_name, type) \
	INIT_STATIC_PROTOPACKET(class_name, type)

#define DECLARE_SERVICE_TEMPL(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, serviceDef::ServiceTempl, serviceDef::ServiceTemplManager)


///
/// DeliverTaskSrvTempl: 发任务
///
class DeliverTaskSrvTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(DeliverTaskSrvTempl, SRV_DEF_DELIVER_TASK);
public:
	static const int kMaxTaskNumber = 128;

public:
	BoundArray<TaskID, kMaxTaskNumber> deliver_task_list; // 发放任务列表
	PACKET_DEFINE(deliver_task_list);

protected:
	virtual bool OnCheckDataValidity() const 
	{ 
		if (deliver_task_list.size() == 0)
			return false;

		for (size_t i = 0; i < deliver_task_list.size(); ++i)
		{
			if (deliver_task_list[i] <= 0)
				return false;
		}

		return true; 
	}
};

///
/// RecycleTaskSrvTempl: 收任务
///
class RecycleTaskSrvTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(RecycleTaskSrvTempl, SRV_DEF_RECYCLE_TASK);
public:
	static const int kMaxTaskNumber = 128;

public:
	BoundArray<TaskID, kMaxTaskNumber> recycle_task_list; // 接收任务列表（验证完成任务）
	PACKET_DEFINE(recycle_task_list);

protected:
	virtual bool OnCheckDataValidity() const 
	{ 
		if (recycle_task_list.size() == 0)
			return false;

		for (size_t i = 0; i < recycle_task_list.size(); ++i)
		{
			if (recycle_task_list[i] <= 0)
				return false;
		}

		return true; 
	}
};

///
/// ShopSrvTempl: 商店服务
///
class ShopSrvTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(ShopSrvTempl, SRV_DEF_SHOP);
public:
	dataTempl::TemplID shop_id;
	PACKET_DEFINE(shop_id);

protected:
	virtual bool OnCheckDataValidity() const
	{
		if (shop_id <= 0)
			return false;
		return true;
	}
};

///
/// StorageTaskSrvTempl: 库任务服务模板
///
class StorageTaskSrvTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(StorageTaskSrvTempl, SRV_DEF_STORAGE_TASK);
public:
	int32_t task_storage_id;
	PACKET_DEFINE(task_storage_id);

protected:
	virtual bool OnCheckDataValidity() const 
	{ 
		if (task_storage_id > 0)
			return true;

		return false; 
	}
};

class EnhanceSrvTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(EnhanceSrvTempl, SRV_DEF_ENHANCE);
public:
	int32_t enhance_gid;                 // 附魔库ID
	PACKET_DEFINE(enhance_gid);

protected:
	virtual bool OnCheckDataValidity() const 
	{ 
        return enhance_gid > 0;
	}
};

///
/// SimpleServiceTempl
///
class SimpleServiceTempl : public ServiceTempl
{
	DECLARE_SERVICE_TEMPL(SimpleServiceTempl, SRV_DEF_SIMPLE_SERVICE);
public:
	static const int kMaxSimpleServiceCount = 128;
	BitSet<kMaxSimpleServiceCount> service_mask;
	PACKET_DEFINE(service_mask);

protected:
	virtual bool OnCheckDataValidity() const { return true; }
};

} // namespace serviceDef

#endif // GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_DEF_H_
