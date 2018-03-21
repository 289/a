#ifndef SKILL_CAST_PROC_H_
#define SKILL_CAST_PROC_H_

#include "skill_types.h"

namespace skill
{

class AttackFrame;
class InnerMsg;
class SkillTempl;
class FrameDamage;

class CastProc
{
public:
	CastProc(Player* attacker, Player* trigger, const AttackFrame* frame, FrameDamage* dmg, InnerMsg* msg);

	void Cast(const SkillTempl* skill, const PlayerVec& target);
	bool CastNormalEffect(const SkillTempl* skill, const PlayerVec& target);
	void CastRedirEffect();
private:
	Player* attacker_;
	Player* trigger_;
	const AttackFrame* frame_;
	FrameDamage* dmg_;
	InnerMsg* msg_;
};

} // namespace skill

#endif // SKILL_CAST_PROC_H_
