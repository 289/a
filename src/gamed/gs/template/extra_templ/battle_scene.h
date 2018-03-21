#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_BATTLE_SCENE_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_BATTLE_SCENE_H_

#include "base_extratempl.h"


namespace extraTempl {

class BattleSceneTempl : public BaseExtraTempl
{
	DECLARE_EXTRATEMPLATE(BattleSceneTempl, TEMPL_TYPE_BATTLE_SCENE_TEMPL);
public:
    static const int kMaxMusicPath = UTF8_LEN(128);

	inline void set_templ_id(TemplID id) { templ_id = id; }
	virtual std::string TemplateName() const { return "battle_scene"; }

	enum MIRROR_TYPE
	{
		MIRROR_TYPE_NONE = 0,
		MIRROR_TYPE_MIRROR,
		MIRROR_TYPE_RANDOM,
	};


public:
// 0
	BoundArray<int8_t, 32> scene_name; // 场景名字
	TemplID  scene_data_id;            // 场景资源ID
	EventID  scene_event_id;           // 场景的AI，用于挂策略的lua脚本。默认值0表示没有事件策略，有值则必须与templ_id一致
	int8_t   scene_pos_layout;         // 场景站位布局，0: 默认站位;
	int8_t   mirror_type;              // 对应枚举MIRROR_TYPE，场景镜像类型。默认值0表示没有镜像 

// 5
    BoundArray<uint8_t, kMaxMusicPath> combat_win_music;  // 战斗胜利音乐
    BoundArray<uint8_t, kMaxMusicPath> combat_fail_music; // 战斗失败音乐


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(scene_name, scene_data_id, scene_event_id, scene_pos_layout, mirror_type);
        MARSHAL_TEMPLVALUE(combat_win_music, combat_fail_music);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(scene_name, scene_data_id, scene_event_id, scene_pos_layout, mirror_type);
        UNMARSHAL_TEMPLVALUE(combat_win_music, combat_fail_music);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (scene_data_id <= 0 || scene_event_id < 0 || scene_pos_layout < 0)
			return false;

		// script id
		if (scene_event_id < 0)
			return false;
		if (scene_event_id != 0 && scene_event_id != templ_id)
			return false;

		if (mirror_type != MIRROR_TYPE_NONE && 
			mirror_type != MIRROR_TYPE_MIRROR &&
			mirror_type != MIRROR_TYPE_RANDOM)
		{
			return false;
		}

		return true;
	}
};

} // namespace extraTempl

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_BATTLE_SCENE_H_
