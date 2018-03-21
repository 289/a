#ifndef SKILL_DATATEMPL_CAMERA_MOVE_H_
#define SKILL_DATATEMPL_CAMERA_MOVE_H_

#include "skill_types.h"

namespace skill
{

class CameraMove
{
public:
    CameraMove()
        : move(false), zoom_factor(0), zoom_time(0), zoom_speed(0)
    {
    }

    inline bool CheckDataValidity() const;
    bool move;
    float zoom_factor;
    int32_t zoom_time;
    int32_t zoom_speed;
    std::string gfx_fullscreen;
    NESTED_DEFINE(move, zoom_factor, zoom_time, zoom_speed, gfx_fullscreen);
};

inline bool CameraMove::CheckDataValidity() const
{
    return zoom_factor >= 0 && (zoom_time == -1 || zoom_time >= 0) && zoom_speed >= 0;
}
typedef std::vector<CameraMove> CameraMoveVec;

} // namespace skill

#endif // SKILL_DATATEMPL_CAMERA_MOVE_H_
