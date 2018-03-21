#ifndef GAMED_GS_OBJ_SERVICE_PROVIDER_H_
#define GAMED_GS_OBJ_SERVICE_PROVIDER_H_

#include <unistd.h>

#include "shared/base/copyable.h"
#include "shared/net/packet/bit_set.h"
#include "shared/net/packet/packet.h"
#include "gs/global/game_types.h"
#include "gs/global/time_slot.h"
#include "gs/template/data_templ/service_templ.h"


namespace gamed {

typedef shared::net::ProtoPacket& PacketRef;

class WorldObject;

#define DEFINE_PROVIDER(class_name, type) \
protected: \
	virtual ServiceProvider* Clone()\
	{ \
		return static_cast<ServiceProvider*>(new class_name(*this)); \
	} \
	class_name(const class_name& rhs) \
		: ServiceProvider(rhs.type_) \
	{ } \
public: \
	class_name() \
		: ServiceProvider(type) \
	{ } \


/**
 * @brief ServiceProvider: Npc端的服务判定，如果是UI服务则在Player进行判定
 *    （1）每个服务在每个Npc里都保存一个实例，UI服务则在player身上保存
 *    （2）所有的ServiceProvider都是copyable的
 *    （3）新声明的provider子类都需要在ServiceManager里做define才能生效
 */
class ServiceProvider : public shared::copyable
{
	friend class ServiceManager;
public:
	ServiceProvider(int type);
	virtual ~ServiceProvider();

	virtual ServiceProvider* Clone()  = 0;
	bool Init(WorldObject* obj, const serviceDef::ServiceTempl* param_tmpl);
	void PayService(const XID& player, const void* buf, size_t size);
	void GetServiceContent(const XID& player, int link_id, int client_sid);
	
	inline int GetType() const;

protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl) = 0;
	virtual void TryServe(const XID& player, const void* buf, size_t size) = 0;
	virtual void GetContent(const XID& player, int link_id, int client_sid) {ASSERT(false);}

	void SendCmd(int64_t roleid, int32_t link_id, int32_t client_sid, PacketRef packet) const;
	void SendMessage(int message, const XID& target, const void* buf, size_t size) const;
	void SendMessage(int message, const XID& target, int error, const void* buf=NULL, size_t size=0) const;
	void SendServiceContent(int64_t roleid, int32_t link_id, int32_t client_sid, const void *buf, size_t size) const;

protected:
	int          type_;

private:
	WorldObject* pobj_;
	bool         is_inited_;
};

///
/// inline func
///
inline int ServiceProvider::GetType() const
{
	return type_;
}


///
/// DeliverTaskProvider: 发放任务
///
class DeliverTaskProvider : public ServiceProvider
{
	DEFINE_PROVIDER(DeliverTaskProvider, serviceDef::SRV_DEF_DELIVER_TASK);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);

private:
	typedef std::set<serviceDef::TaskID> TaskIDSet;
	TaskIDSet deliver_task_set_;
};


///
/// RecycleTaskProvider: 回收任务 ---- player交任务
/// 
class RecycleTaskProvider : public ServiceProvider
{
	DEFINE_PROVIDER(RecycleTaskProvider, serviceDef::SRV_DEF_RECYCLE_TASK);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);

private:
	typedef std::set<serviceDef::TaskID> TaskIDSet;
	TaskIDSet recycle_task_set_;
};

///
/// ShopProvider: 商店功能
///
class ShopProvider : public ServiceProvider
{
	DEFINE_PROVIDER(ShopProvider, serviceDef::SRV_DEF_SHOP);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);
	virtual void GetContent(const XID& player, int link_id, int client_sid);

private:
	void UpdateGoods();

private:
	/*货币类型*/
	enum MONEY_TYPE
	{
		MT_INVALID,
		MT_MONEY,
		MT_CASH,
		MT_SCORE,
		MT_MAX,
	};
	/*刷新规则*/
	enum REFRESH_RULE
	{
		REFRESH_INVALID,
		REFRESH_EMPTY,    // 不刷新
		REFRESH_PER_DAY,  // 每天凌晨刷新
		REFRESH_MAX,
	};
	/*商品信息*/
	struct Goods
	{
		int32_t item_id;
		int32_t init_item_count;
		int32_t item_remains;
		int32_t pile_limit;
		int32_t sell_price;
		int16_t refresh_rule;
		int16_t refresh_interval;
		int32_t refresh_timestamp;
	};

	typedef std::vector<Goods> GoodsVec;
	typedef std::vector<int/*goods-idx*/> RefreshVec;
	GoodsVec    goods_list_;        // 商品列表
	RefreshVec  refresh_list_;      // 需要刷新的商品
	TimeSlot    open_time_;         // 商店开启时间
	int16_t     coin_type_;         // 商店货币类型
	int16_t     total_goods_count_; // 商品总数
	int16_t     dyn_goods_count_;   // 动态商品个数
};

///
/// StorageTaskProvider: 库任务服务
///
class StorageTaskProvider : public ServiceProvider
{
	DEFINE_PROVIDER(StorageTaskProvider, serviceDef::SRV_DEF_STORAGE_TASK);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);

private:
	int32_t task_storage_id_; // 任务库的id
};

///
/// EnhanceProvider: 附魔服务
///
class EnhanceProvider : public ServiceProvider
{
	DEFINE_PROVIDER(EnhanceProvider, serviceDef::SRV_DEF_ENHANCE);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);
	virtual void GetContent(const XID& player, int link_id, int client_sid);
private:
    void UpdateEnhanceInfo();
private:
	int32_t enhance_gid_; // 附魔库id
    int32_t count_;
    int32_t reset_time_;
};

///
/// SimpleServiceProvider：简单服务
///
class SimpleServiceProvider : public ServiceProvider
{
	DEFINE_PROVIDER(SimpleServiceProvider, serviceDef::SRV_DEF_SIMPLE_SERVICE);
protected:
	virtual bool OnInit(const serviceDef::ServiceTempl* param_tmpl);
	virtual void TryServe(const XID& player, const void* buf, size_t size);

private:
	shared::net::BitSet<serviceDef::SimpleServiceTempl::kMaxSimpleServiceCount> service_mask;
};

#undef DEFINE_PROVIDER

} // namespace gamed

#endif // GAMED_GS_OBJ_SERVICE_PROVIDER_H_
