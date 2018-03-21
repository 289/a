#include "shared/net/buffer.h"
#include "shared/security/randomgen.h"
#include "skill_info.h"
#include "obj_interface.h"
#include "skill_templ_manager.h"
#include "skill_templ.h"
#include "model_action.h"
#include "effect_templ.h"
#include "logic_if.h"

namespace skill
{

using namespace std;
using namespace shared::net;

static const int32_t    TIME_MOVEBACK       = 130;
static const int32_t    TIME_RECAST_MOVE    = 200;
static const int32_t    SCREEN_MAX_LEN      = 1000;         //单位是像素
static const int32_t    TIME_ATTACKED       = 1300;
static const int32_t    PET_CAST_TIME       = 2500;
//static const int32_t    PORTRAIT_TIME       = 1200;
static const int32_t    PORTRAIT_TIME       = 0;
static const int32_t    TIME_ERROR          = 5000;       // 很大的时间，提示技能有误(先改成5秒避免卡主战斗)
static const string     DEFAULT_ACTION      = "普通攻击";
static const string     DEFAULT_GFX         = "站立";

static SkillTempl skill_templ;
static EffectTempl effect_templ;
static ModelActionConf model_action;

bool InitSkillData(const char* file)
{
	RandomGen::Init();
	LogicIf::Init();
	return s_pSkillTempl->ReadFromFile(file);
}

static bool InitActionData(const string& data)
{
    model_action.model_action_map.clear();
    ByteBuffer byte_buff;
    byte_buff.append(data.c_str(), data.length());
    model_action.UnPack(byte_buff);
    return true;
}

bool InitActionData(const char* file)
{
	FILE* pfile       = NULL;
	int32_t file_size = 0;
	int result        = 0;

	if ((pfile = fopen(file, "rb")) == NULL)
		return false;

	// 获取文件大小
	fseek(pfile, 0L, SEEK_END);
	file_size = ftell(pfile);
	rewind(pfile);

    string data;
    data.resize(file_size);

	// 拷贝整个文件
	result = fread(&(data[0]), 1, file_size, pfile);
	if (result != file_size)
	{
		fclose(pfile);
		return false;
	}

    InitActionData(data);

	fclose(pfile);
	return true;
}

int32_t GetATBStopTime(SkillID id)
{
    return GetSkillTempl(id)->atb_stop_time;
}

static bool IsPet(const string& model)
{
    int32_t prefix_len = 21;
    int32_t subfix_len = 4;
    string tmp = model.substr(prefix_len, model.length() - prefix_len - subfix_len);
    return tmp.empty();
}

static ActionInfo* GetAction(const string& model, const string& action)
{
    assert(!action.empty());
    // 宠物释放技能时没有模型，不允许宠物使用动作
    if (IsPet(model))
    {
        __PRINTF("GetAction model is empty");
        return NULL;
    }
    // 如果找不到模型
    ModelActionMap::iterator mit = model_action.model_action_map.find(model);
    if (mit == model_action.model_action_map.end())
    {
        __PRINTF("GetAction model=%s not find", model.c_str());
        return NULL;
    }
    // 查找对应动作的时间
    ActionInfoMap& action_map = mit->second;
    for (int32_t i = 0; i < 2; ++i)
    {
        const string& path = i == 0 ? action : DEFAULT_ACTION;
        ActionInfoMap::iterator ait = action_map.find(path);
        if (ait != action_map.end())
        {
            return &(ait->second);
        }
    }
    __PRINTF("GetAction model=%s action=%s not find", model.c_str(), action.c_str());
    return NULL;
}

static void ReplaceBlackSlash(string& str)
{
    string blackslash = "\\";
    string::size_type pos = 0;
    while (true)
    {
        if ((pos = str.find(blackslash)) == string::npos)
        {
            break;
        }
        str.replace(pos, blackslash.length(), "/");
    }
}

static ActionInfo* GetGfx(const string& model, const string& gfx)
{
    assert(!model.empty() && !gfx.empty());
    string path = "/assets/config/model/" + gfx;
    ReplaceBlackSlash(path);
    ModelActionMap::iterator mit = model_action.model_action_map.find(path);
    if (mit == model_action.model_action_map.end())
    {
        __PRINTF("GetGfx gfx=%s not find", gfx.c_str());
        return NULL;
    }
    ActionInfoMap& action_map = mit->second;
    ActionInfoMap::iterator ait = action_map.find(DEFAULT_GFX);
    return ait == action_map.end() ? NULL : &(ait->second);
}

static int32_t GetCastAction(const string& model, const SkillTempl* templ)
{
    // 如果没填动作则直接跳过
    if (templ->cast_path.empty())
    {
        return 0;
    }

    ActionInfo* info = GetAction(model, templ->cast_path);
    return info == NULL ? TIME_ERROR : info->GetTotalTime();
}

static int32_t GetCastGfx(const string& model, const SkillTempl* templ)
{
    // 宠物释放技能，返回一个固定的时间
    if (IsPet(model))
    {
        return PET_CAST_TIME;
    }
    // 没有光效，直接跳过
    if (templ->cast_path.empty())
    {
        return 0;
    }
    ActionInfo* info = GetGfx(model, templ->cast_path);
    return info == NULL ? TIME_ERROR : info->GetTotalTime();
}

static int32_t GetCastTime(const string& model, const SkillTempl* templ)
{
    return templ->cast_type == CAST_ACTION ? GetCastAction(model, templ) : GetCastGfx(model, templ);
}

static int32_t GetMoveTime(const string& model, const SkillTempl* templ)
{
    // 如果没填动作则直接跳过
    if (templ->attack_path.empty())
    {
        return 0;
    }
    ActionInfo* info = GetAction(model, templ->attack_path);
    return info == NULL ? TIME_ERROR : info->GetDurationTime(0, templ->move_frame);
}

static int32_t GetSkillTime(const string& model, const SkillTempl* templ)
{
    // 如果没填动作则直接跳过
    if (templ->attack_path.empty())
    {
        return 0;
    }

    ActionInfo* info = GetAction(model, templ->attack_path);
    if (info == NULL)
    {
        return TIME_ERROR;
    }
    if (templ->move_frame >= info->frame_num)
    {
        __PRINTF("skillid=%d model=%s move_frame=%d frame_num=%d", templ->templ_id, model.c_str(), templ->move_frame, info->frame_num);
        return TIME_ERROR;
    }
    return info->GetDurationTime(templ->move_frame, info->frame_num);
}

static int32_t GetFrameAction(const string& model, const SkillTempl* templ, int32_t index)
{
    // 如果没填动作则直接跳过
    if (templ->attack_path.empty())
    {
        return 0;
    }

    ActionInfo* info = GetAction(model, templ->attack_path);
    if (info == NULL)
    {
        return TIME_ERROR;
    }
    int32_t attack_frame = info->GetAttackFrame(index);
    return info->GetDurationTime(templ->move_frame, attack_frame);
}

static int32_t GetFrameGfx(const string& model, const SkillTempl* templ, int32_t index)
{
    // 没有光效，直接跳过
    if (templ->attack_gfx_path.empty())
    {
        return 0;
    }
    ActionInfo* info = GetGfx(model, templ->attack_gfx_path);
    if (info == NULL)
    {
        return TIME_ERROR;
    }
    int32_t attack_frame = info->GetAttackFrame(index);
    return info->GetDurationTime(0, attack_frame);
}

static int32_t GetFrameTime(const string& model, const SkillTempl* templ, int32_t index)
{
    int32_t time = GetFrameAction(model, templ, index);
    time += GetFrameGfx(model, templ, index);
    return time;
}

static int32_t GetEffectTime(const string& model, const PlayerDamage& dmg)
{
    ActionInfo* info = NULL;
    const EffectDamageVec& effect_vec = dmg.dmgs;
    // 如果effect_vec为空，表示miss，这时只需要计算一个闪避动作时间
    int32_t max_time = effect_vec.empty() ? TIME_ATTACKED : 0;
    EffectDamageVec::const_iterator eit = effect_vec.begin();
    for (; eit != effect_vec.end(); ++eit)
    {
        int32_t time = 0;
        const EffectTempl* effect = GetEffectTempl(eit->effectid);
        if (effect->attack)
        {
            if (effect->bullet.speed != 0)
            {
                time += SCREEN_MAX_LEN * 1000 / effect->bullet.speed;
            }
            time += TIME_ATTACKED;
            if (effect->bullet.bullet_action())
            {
                info = GetAction(model, effect->bullet.path);
                if (info == NULL)
                {
                    time += TIME_ERROR;
                }
                else
                {
                    int32_t move_frame = effect->bullet.move_frame;
                    time += info->GetDurationTime(move_frame, info->GetAttackFrame());
                }
            }
        }
        if (max_time < time)
        {
            max_time = time;
        }
    }
    return max_time;
}

static void GetNormalDefendPause(const string& model, int32_t extra, const PlayerDamageVec& players, PauseTime& pause)
{
	PlayerDamageVec::const_iterator pit = players.begin();
	for (; pit != players.end(); ++pit)
	{
		int32_t& time = pause[pit->defender];
		int32_t attacked_time = GetEffectTime(model, *pit);
		if (time < (extra + attacked_time))
		{
			time = (extra + attacked_time);
		}
	}
}

static void GetNormalPause(const string& model, int32_t index, const SkillDamageVec& dmg, PauseTime& pause)
{
	const SkillDamage& skill_dmg = dmg[index];
	const SkillTempl* templ = GetSkillTempl(skill_dmg.skillid);

    // 1、原地动作/原地光效时间
    int32_t cast = GetCastTime(model, templ);
    // 2、移动时间
    int32_t move = index == 0 && templ->action == ACTION_MELEE ? GetMoveTime(model, templ) : 0;
    int32_t back = index == 0 && templ->action == ACTION_MELEE ? TIME_MOVEBACK : 0;
    int32_t recast = index != 0 && dmg[index - 1].cast_pos != dmg[index].cast_pos ? TIME_RECAST_MOVE : 0;
    // 3、技能动作时间
    const FrameDamageVec& frame_vec = skill_dmg.frames;
    assert(frame_vec.size() >= 1);
    int32_t action = GetSkillTime(model, templ);
    pause[skill_dmg.attacker] += cast + move + back + recast + action;

    // 4、被攻击者需要暂停的时间
    int32_t waiting = move + cast + recast;
    for (size_t i = 0; i < frame_vec.size(); ++i)
    {
        int32_t extra_time = GetFrameTime(model, templ, i) + waiting;
		const FrameDamage& frame = frame_vec[i];
        GetNormalDefendPause(model, extra_time, frame.players, pause);
        GetNormalDefendPause(model, extra_time, frame.redir_players, pause);
    }
}

static int32_t GetSpecSkillTime(const string& model, const SkillTempl* templ)
{
    // 如果没填动作则直接跳过
    if (templ->attack_path.empty())
    {
        return 0;
    }

    ActionInfo* info = GetAction(model, templ->attack_path);
    if (info == NULL)
    {
        return TIME_ERROR;
    }
    int32_t frame_num = templ->action == ACTION_MELEE ? 1 : info->frame_num;
    return info->GetDurationTime(0, frame_num);
}

static void GetSpecPause(const string& model, int32_t index, const SkillDamageVec& dmg, PauseTime& pause)
{
	const SkillDamage& skill_dmg = dmg[index];
	const SkillTempl* templ = GetSkillTempl(skill_dmg.skillid);

    // 1、原地动作/原地光效时间
    int32_t cast = GetCastTime(model, templ);
    // 2、移动时间
    int32_t move = index == 0 && templ->action == ACTION_MELEE ? GetMoveTime(model, templ) : 0;
    int32_t back = index == 0 && templ->action == ACTION_MELEE ? TIME_MOVEBACK : 0;
    int32_t recast = index != 0 && dmg[index - 1].cast_pos != dmg[index].cast_pos ? TIME_RECAST_MOVE : 0;

    // 3、技能动作时间
    int32_t action = GetSpecSkillTime(model, templ);
    const FrameDamageVec& frame_vec = skill_dmg.frames;
    assert(frame_vec.size() == 1);
	const PlayerDamageVec& player_vec = frame_vec[0].players;
    int32_t tmp = action + 300 * (player_vec.size() - 1);
    pause[skill_dmg.attacker] += cast + move + back + recast + tmp;

    // 4、被攻击者需要暂停的时间
    int32_t defend = move + cast + recast + tmp + TIME_ATTACKED;
	PlayerDamageVec::const_iterator pit = player_vec.begin();
	for (; pit != player_vec.end(); ++pit)
	{
        pause[pit->defender] = defend;
	}
}

void GetPauseTime(const string& model, const SkillDamageVec& dmg, PauseTime& pause)
{
    pause.clear();
    string path = "/assets/config/model";
    if (model[0] != '/')
    {
        path += "/";
    }
    path += model + ".xml";
    for (size_t index = 0; index < dmg.size(); ++index)
    {
        const SkillTempl* templ = GetSkillTempl(dmg[index].skillid);
        if (templ->use_portrait())
        {
            pause[dmg[index].attacker] += PORTRAIT_TIME;
        }
        if (templ->range.type == RANGE_CHAIN)
        {
            GetSpecPause(path, index, dmg, pause);
        }
        else
        {
            GetNormalPause(path, index, dmg, pause);
        }
    }
}

} // namespace skill
