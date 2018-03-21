#ifndef SKILL_DATATEMPL_BULLET_H_
#define SKILL_DATATEMPL_BULLET_H_

#include "skill_types.h"

namespace skill
{

class Bullet
{
public:
    Bullet()
        : type(BULLET_GFX), speed(0), move_frame(0)
    {
    }

    inline bool CheckDataValidity() const;
    inline bool bullet_action() const;
    inline bool trigger_action() const;

    int8_t type;
    int32_t speed;
    int32_t move_frame;
    std::string path;

    NESTED_DEFINE(type, speed, move_frame, path);
};

inline bool Bullet::CheckDataValidity() const
{
    CHECK_INRANGE(type, BULLET_GFX, (BULLET_ACTION | BULLET_TRIGGER_ACTION))
    return speed >= 0 && move_frame >= 0;
}

inline bool Bullet::bullet_action() const
{
    return type & BULLET_ACTION;
}

inline bool Bullet::trigger_action() const
{
    return type & BULLET_TRIGGER_ACTION;
}

} // namespace skill

#endif // SKILL_DATATEMPL_BULLET_H_
