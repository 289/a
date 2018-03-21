#ifndef SKILL_BUFF_H_
#define SKILL_BUFF_H_

#include "damage_msg.h"

namespace skill
{

class EffectTempl;
class InnerMsg;

class Buff
{
public:
	Buff();
	Buff(Player* attacker, Player* defender, const EffectTempl* effect);

    // 提供给战斗系统使用
    int32_t GetAttacherId() const;
    int32_t GetTargetId() const;
    int32_t GetBuffSeq() const;
    int32_t GetBuffId() const;

	inline bool IsTimeout();
	inline void DecDuration();
	inline void DecActiveNum();

	void GetTarget(InnerMsg& msg);
	void RoundStart(InnerMsg& msg);
	void RoundEnd(InnerMsg& msg);
	void NormalShield(InnerMsg& msg);
	void ReboundShield(InnerMsg& msg);
	void LifeChain(InnerMsg& msg);
	void ConsumeRevise(InnerMsg& msg);
	void CastRevise(InnerMsg& msg);
	void AttackedRevise(InnerMsg& msg);
	void Attach(InnerMsg& msg);
	void Enhance(InnerMsg& msg);
	void Detach(InnerMsg& msg);
	void BeAttacked(InnerMsg& msg);
public:
	Player* caster_;
	Player* owner_;
	const EffectTempl* effect_;
	uint32_t sn_;
	int32_t num_;
	int32_t overlap_;
	int32_t hp_;
	int64_t duration_;
private:
	static uint32_t next_sn_;
};
typedef std::vector<Buff> BuffVec;

// 内联函数
inline bool Buff::IsTimeout()
{
	return duration_ == 0 || num_ == 0 || hp_ < 0;
}

inline void Buff::DecDuration()
{
	if (duration_ > 0)
	{
		--duration_;
	}
}

inline void Buff::DecActiveNum()
{
	if (num_ > 0)
	{
		--num_;
	}
}

} // namespace skill

#endif // SKILL_BUFF_H_
