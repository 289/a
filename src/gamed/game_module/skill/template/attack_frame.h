#ifndef SKILL_DATATEMPL_ATTACK_FRAME_H_
#define SKILL_DATATEMPL_ATTACK_FRAME_H_

#include "skill_types.h"

namespace skill
{

struct FrameInfo
{
    FrameInfo()
        : effect_id(0), show_count(0), show_interval(0)
    {
    }

	inline bool CheckDataValidity() const;

    EffectID effect_id;
    int32_t show_count;
    int32_t show_interval;

    NESTED_DEFINE(effect_id, show_count, show_interval)
};
typedef std::vector<FrameInfo> FrameVec;

class AttackFrame
{
public:
	inline bool CheckDataValidity() const;

    FrameVec normal;
    FrameVec redir;
	NESTED_DEFINE(normal, redir);

	// 只在内存中初始化
	EffectTemplVec effect_templ;
	EffectTemplVec redir_templ;
};

inline bool FrameInfo::CheckDataValidity() const
{
    return effect_id >= 0 && show_count >= 0 && show_interval >= 0;
}

inline bool AttackFrame::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(normal)
    CHECK_VEC_VALIDITY(redir)
	return true;
}
typedef std::vector<AttackFrame> AttackFrameVec;

} // namespace skill

#endif // SKILL_DATATEMPL_ATTACK_FRAME_H_
