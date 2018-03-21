#ifndef SKILL_LOGIC_IF_H_
#define SKILL_LOGIC_IF_H_

#include "skill_types.h"

namespace skill
{

class EffectTempl;
class ActivePoint;
class EffectLogic;
class InnerMsg;

// 效果逻辑接口类
class LogicIf
{
public:
	static void Init();
	static LogicIf* GetIf(const ActivePoint* point, const EffectLogic* logic);

	virtual bool Init(InnerMsg& msg) const;
	virtual bool GetTarget(InnerMsg& msg) const;
	virtual bool ConsumeRevise(InnerMsg& msg) const;
	virtual bool CastRevise(InnerMsg& msg) const;
	virtual bool AttackedRevise(InnerMsg& msg) const;
	virtual bool Cast(InnerMsg& msg) const;
	virtual bool RoundStart(InnerMsg& msg) const;
	virtual bool RoundEnd(InnerMsg& msg) const;
	virtual bool NormalShield(InnerMsg& msg) const;
	virtual bool ReboundShield(InnerMsg& msg) const;
	virtual bool LifeChain(InnerMsg& msg) const;
	virtual bool Dying(InnerMsg& msg) const;
	virtual bool BeAttacked(InnerMsg& msg) const;
	virtual bool Purify(InnerMsg& msg);
	virtual bool Attach(InnerMsg& msg);
	virtual bool Enhance(InnerMsg& msg);
	virtual bool Detach(InnerMsg& msg);
private:
	typedef std::map<int8_t, LogicIf*> LogicIfMap;
	static LogicIfMap s_logic_map_;
protected:
	int32_t overlap_;
	Player* caster_;
	Player* trigger_;
	const EffectTempl* effect_;
	const EffectLogic* logic_;
};

} // namespace skill

#endif // SKILL_LOGIC_IF_H_
