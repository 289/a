#ifndef SKILL_REDIR_UTIL_H_
#define SKILL_REDIR_UTIL_H_

#include "skill_types.h"

namespace skill
{

class Redirect;

// 重定向目标选择
class RedirUtil
{
public:
	static int8_t GetTarget(Player* caster, Player* trigger, const Redirect& redir, PlayerVec& target);
private:
	static void AllAlive(Player* caster, const Redirect& redir, PlayerVec& target);
	static void RandTarget(Player* caster, const Redirect& redir, PlayerVec& target);
};

} // namespace skill

#endif // SKILL_REDIR_UTIL_H_
