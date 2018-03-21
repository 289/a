#ifndef GAMED_GS_OBJ_SERVICE_EXECUTOR_H_
#define GAMED_GS_OBJ_SERVICE_EXECUTOR_H_

#include "gs/global/game_types.h"
#include "gs/template/data_templ/service_templ.h"


namespace gamed {

class Player;

#define DEFINE_EXECUTOR(class_name, type) \
public: \
	class_name() \
		: ServiceExecutor(type) \
	{ } \
protected: \
	class_name(const class_name& rhs) \
		: ServiceExecutor(rhs.type_) \
	{ } \


/**
 * @brief ServiceExecutor: player端的服务执行
 *    （1）整个GS只有一份，因此所有的子类都不保存中间状态
 *    （2）新声明的executor子类都需要在ServiceManager里做define才能生效
 */
class ServiceExecutor
{
	friend class ServiceManager;
public:
	ServiceExecutor(int type);
	virtual ~ServiceExecutor();

	/**
	 * @brief ServeRequest 
	 *    收到client的cmd后，调用ServeRequest可以先做一些预先检查
	 */
	bool ServeRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;

	/**
	 * @brief Serve 
	 *    收到provider的回复时调用
	 */
	bool Serve(Player* pplayer, const XID& provider, const void* buf, size_t size) const;

	/**
	 * @brief CheckData 
	 *    检查收到client的cmd里，参数是否正确
	 */
	bool CheckRequestParam(const void* buf, size_t size) const;

	inline int GetType() const;

protected:
	virtual bool CheckSrvParam(const void* buf, size_t size) const = 0;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const = 0;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const = 0;

protected:
	int    type_;
};

///
/// inline func
///
inline int ServiceExecutor::GetType() const
{
	return type_;
}


///
/// DeliverTaskExecutor: 发放任务
///
class DeliverTaskExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(DeliverTaskExecutor, serviceDef::SRV_DEF_DELIVER_TASK);
protected:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};


///
/// RecycleTaskExecutor: 回收任务 ---- player交任务
///
class RecycleTaskExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(RecycleTaskExecutor, serviceDef::SRV_DEF_RECYCLE_TASK);
protected:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};


///
/// ShopExecutor: 店铺服务
///
class ShopExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(ShopExecutor, serviceDef::SRV_DEF_SHOP);
public:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};

///
/// StorageTaskExecutor: 库任务服务
///
class StorageTaskExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(StorageTaskExecutor, serviceDef::SRV_DEF_STORAGE_TASK);
public:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};

///
/// EnhanceExecutor: 附魔服务
///
class EnhanceExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(EnhanceExecutor, serviceDef::SRV_DEF_ENHANCE);
public:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};

///
/// SimpleServiceExecutor: 简单服务
///
class SimpleServiceExecutor : public ServiceExecutor
{
	DEFINE_EXECUTOR(SimpleServiceExecutor, serviceDef::SRV_DEF_SIMPLE_SERVICE);
protected:
	virtual bool CheckSrvParam(const void* buf, size_t size) const;
	virtual bool SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
	virtual bool OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const;
};


#undef DEFINE_EXECUTOR

} // namespace gamed

#endif // GAMED_GS_OBJ_SERVICE_EXECUTOR_H_
