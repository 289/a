#ifndef TASK_BASE_TASK_TEMPL_H_
#define TASK_BASE_TASK_TEMPL_H_

#include "task_types.h"

namespace task
{

typedef int32_t TemplID;
typedef uint16_t TemplType;

enum TaskTemplType
{
	TEMPL_TYPE_TASK,
	TEMPL_TYPE_STORAGE,
	TEMPL_TYPE_MONSTER_PACKAGE,
	TEMPL_TYPE_RATIO_TABLE,
};

class BaseTempl : public shared::net::BasePacket
{
public:
	BaseTempl(TemplType type);
	virtual ~BaseTempl();

	virtual void Marshal();
	virtual void Unmarshal();
	virtual bool CheckDataValidity() const;
protected:
	virtual void OnMarshal() = 0;
	virtual void OnUnmarshal() = 0;
	virtual bool OnCheckDataValidity() const = 0;
public:
	TemplID id;
};

class BaseTemplCreater : public shared::Singleton<BaseTemplCreater>
{
	friend class shared::Singleton<BaseTemplCreater>;
public:
	static inline BaseTemplCreater* GetInstance()
	{
		return &(get_mutable_instance());
	}

	static BaseTempl* CreatePacket(TemplType type);
	static bool InsertPacket(TemplType type, BaseTempl* templ);
	static bool IsValidType(int32_t type);
protected:
	BaseTemplCreater();
	~BaseTemplCreater();

	bool OnInsertPacket(TemplType type, BaseTempl* templ);
	BaseTempl* OnCreatePacket(TemplType type);
private:
	typedef std::map<TemplType, BaseTempl*> BaseTemplMap;
	BaseTemplMap templ_map_;
};

#define DECLARE_SYS_TEMPL(name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(name, type, task::BaseTempl, task::BaseTemplCreater)

#define INIT_STATIC_SYS_TEMPL(name, type) \
	INIT_STATIC_PROTOPACKET(name, type)

#define MARSHAL_SYS_TEMPL_VALUE(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_SYS_TEMPL_VALUE(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

} // namespace task

#endif // TASK_BASE_TASK_TEMPL_H_
