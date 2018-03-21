#ifndef GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_NPC_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_NPC_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class ServiceNpcTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(ServiceNpcTempl, TEMPL_TYPE_SERVICE_NPC_TEMPL);
public:
	static const int kMaxVisibleNameLen     = UTF8_LEN(16); // 最多支持16个中文字符
	static const int kMaxTitleLen           = UTF8_LEN(16);
	static const int kMaxModelSrcLen        = 512;
	static const int kMaxIdentifyPicSrcLen  = 512;
	static const int kMaxFullPortrait       = 512;
	static const int kMaxBasePrologue       = 10;   // 最多10条基础开场白
	static const int kMaxPrologueContent    = UTF8_LEN(128); // 一条基础开场白最多支持128个中文字符
	static const int kMaxNpcHideTask        = 64;
	static const int kMaxNpcShowTask        = 64;
    static const int kMaxBubbleTalkCount    = 5;
    static const int kMaxBubbleTalkLen      = UTF8_LEN(128); // 128个汉字

    enum BubbleTalkSeq
    {
        BTS_RAND,   // 随机发言
        BTS_ORDER,  // 顺序发言
    };

    struct BubbleTalkInfo
    {
        int8_t  seq_type; // 对应枚举BubbleTalkSeq，默认值：随机发言
        int16_t duration; // 发言冒泡持续时间, 单位秒，默认值：5
        int16_t interval; // 发言的间隔，单位秒，默认值：5
        BoundArray<BoundArray<uint8_t, kMaxBubbleTalkLen>, kMaxBubbleTalkCount> content; // 发言内容
        NESTED_DEFINE(seq_type, duration, interval, content);
    };

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
// 0 ---- 0~3 only used in client
	BoundArray<uint8_t, kMaxVisibleNameLen>    visible_name;       // 游戏里怪物的显示名
	BoundArray<uint8_t, kMaxTitleLen>          title;              // 头衔，名字上方的显示
	BoundArray<uint8_t, kMaxModelSrcLen>       title_pic;          // 头衔图片，名字上方的显示
	BoundArray<uint8_t, kMaxModelSrcLen>       model_src;          // 模型资源
	BoundArray<uint8_t, kMaxIdentifyPicSrcLen> identify_pic_src;   // 图鉴资源，如头像

// 5 ---- used in both client and server
	BoundArray<uint8_t, kMaxFullPortrait>      full_portrait;      // 全身像，立绘
	BoundArray<BoundArray<uint8_t, kMaxPrologueContent>, kMaxBasePrologue> base_prologue; // 基础开场白，最多10条
	int32_t        prologue_script_id;   /* 开场白脚本id，默认值0表示没有脚本，非零正整数表示有脚本，
	                                      * 有脚本id则必须与自己的templ_id一致
										  * */
	uint8_t        faction;              // 阵营
	std::string    service_content;      // 所挂载的服务的具体内容，详情见service_templ.h

// 10
	int8_t         cat_vision;           // 喵类视觉可见等级，只有玩家的喵类视觉等级大于等于该值，才能看见该npc，默认值0，最大值32
	int8_t         info_display_pos;     // 头顶信息显示位置，默认为0         
	int8_t         initial_visible;      // npc可见条件，默认值1表示该npc初始可见，0表示初始不可见
	BoundArray<int32_t, kMaxNpcHideTask> task_hide_npc; // npc可见条件，完成或携带任务时，npc不可见。优先判断此条
	BoundArray<int32_t, kMaxNpcShowTask> task_show_npc; // npc可见条件，完成或携带任务时，npc可见。

// 15
	int8_t         allow_change_dir;     // 和玩家交互时是否可以转向，默认值1表示允许转向，0表示不能转向
	int8_t         gender;               // 性别，默认值0表示男，1表示女，对应basetempl里的GENDER_TYPE枚举
    std::string    enhance_prologue;     // 附魔开场白
    BubbleTalkInfo bubble_talk;          // npc冒泡对话
    int8_t         cat_vision_hint;      // 是否提示玩家使用喵类视觉功能


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, title, title_pic, model_src, identify_pic_src);
		MARSHAL_TEMPLVALUE(full_portrait, base_prologue, prologue_script_id, faction, service_content);
		MARSHAL_TEMPLVALUE(cat_vision, info_display_pos, initial_visible, task_hide_npc, task_show_npc);
		MARSHAL_TEMPLVALUE(allow_change_dir, gender, enhance_prologue, bubble_talk, cat_vision_hint);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(visible_name, title, title_pic, model_src, identify_pic_src);
		UNMARSHAL_TEMPLVALUE(full_portrait, base_prologue, prologue_script_id, faction, service_content);
		UNMARSHAL_TEMPLVALUE(cat_vision, info_display_pos, initial_visible, task_hide_npc, task_show_npc);
		UNMARSHAL_TEMPLVALUE(allow_change_dir, gender, enhance_prologue, bubble_talk, cat_vision_hint);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (model_src.size() <= 0)
            return false;

		// script
		if (prologue_script_id < 0)
		{
			return false;
		}
		else if (prologue_script_id > 0 && prologue_script_id != templ_id)
		{
			return false;
		}

		if (cat_vision < 0 || cat_vision > 32)
			return false;

		if (info_display_pos < 0)
			return false;

		if (gender != GT_MALE && gender != GT_FEMALE)
			return false;

        if (bubble_talk.content.size() > 0)
        {
            if (bubble_talk.seq_type != BTS_ORDER && bubble_talk.seq_type != BTS_RAND)
                return false;

            if (bubble_talk.duration <= 0 || bubble_talk.interval <= 0)
                return false;
        }

        if (cat_vision_hint != 0 && cat_vision_hint != 1)
            return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_SERVICE_NPC_TEMPL_H_
