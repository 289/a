#ifndef SKILL_DATATEMPL_BASE_TEMPL_H_
#define SKILL_DATATEMPL_BASE_TEMPL_H_

#include "shared/base/singleton.h"
#include "skill_types.h"

namespace skill
{

typedef int32_t TemplID;
typedef uint16_t TemplType;

// 技能系统模板类型
enum SkillTemplType
{
	TEMPL_TYPE_SKILL,
	TEMPL_TYPE_EFFECT,
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
	TemplID templ_id;
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
	static bool InsertPacket(uint16_t type, BaseTempl* templ);
	static bool IsValidType(int32_t type);
protected:
	BaseTemplCreater();
	~BaseTemplCreater();

	bool OnInsertPacket(uint16_t type, BaseTempl* templ);
	BaseTempl* OnCreatePacket(TemplType type);
private:
	typedef std::map<TemplType, BaseTempl*> BaseTemplMap;
	BaseTemplMap templ_map_;
};

#define DECLARE_SKILLSYS_TEMPL(name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(name, type, skill::BaseTempl, skill::BaseTemplCreater)

#define INIT_STATIC_SKILLSYS_TEMPL(name, type) \
	INIT_STATIC_PROTOPACKET(name, type)

#define MARSHAL_SKILLSYS_TEMPL_VALUE(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_SKILLSYS_TEMPL_VALUE(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

} // namespace skill

#endif // SKILL_DATATEMPL_BASE_TEMPL_H_
