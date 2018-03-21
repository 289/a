#ifndef SKILL_DATATEMPL_MODEL_ACTION_H_
#define SKILL_DATATEMPL_MODEL_ACTION_H_

#include <stdint.h>
#include "shared/net/packet/base_packet.h"
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"

namespace skill
{

struct ActionInfo
{
    int32_t frame_num;
    std::vector<int8_t> attack_frame;
    std::vector<int32_t> frame_time;

    NESTED_DEFINE(frame_num, attack_frame, frame_time);

    bool IsAttackFrame(int32_t frame)
    {
        return frame < frame_num && attack_frame[frame];
    }

    int32_t GetAttackFrame(int32_t index = 0)
    {
        int32_t count = 0;
        for (int32_t i = 0; i < frame_num; ++i)
        {
            if (IsAttackFrame(i) && count++ == index)
            {
                return i;
            }
        }
        return frame_num;
    }

    int32_t GetDurationTime(int32_t from, int32_t to)
    {
        if (from < 0 || from > to || to > frame_num)
        {
            return -1;
        }
        int32_t duration = 0;
        for (int32_t i = from; i < to; ++i)
        {
            duration += frame_time[i];
        }
        return duration;
    }

    int32_t GetTotalTime()
    {
        return GetDurationTime(0, frame_num);
    }
};
typedef std::map<std::string, ActionInfo> ActionInfoMap;
typedef std::map<std::string, ActionInfoMap> ModelActionMap;

struct ModelActionConf
{
    ModelActionMap model_action_map;
    NESTED_DEFINE(model_action_map);
};

} // namespace skill

#endif // SKILL_DATATEMPL_MODEL_ACTION_H_
