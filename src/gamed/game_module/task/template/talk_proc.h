#ifndef TASK_TALK_PROC_H_
#define TASK_TALK_PROC_H_

#include "task_types.h"

namespace task
{

enum OptionType
{
	OP_TALK_GOTO,
	OP_TALK_END,
	OP_SHOW_IMAGE,
	OP_TASK_DELIVER,
	OP_TASK_FINISH,
    OP_SHOW_IMAGE_WITHOUT_PORTRAIT,
};

class Option
{
public:
	Option()
		: type(OP_TALK_END)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t type;		// 选项的类型
	std::string text;	// 选项或者图片上的文字
	// OP_TALK_GOTO：对话的索引；OP_TASK_DELIVER：任务ID；OP_SHOW_IMAGE：图片的路径
	std::string param;		

	NESTED_DEFINE(type, text, param);
};
typedef std::vector<Option> OptionVec;

enum TalkPosition
{
	POS_LEFT,
	POS_RIGHT,
};

enum SpeakerType
{
	SPEAKER_PLAYER,
	SPEAKER_NPC,
};

enum EmotionType
{
	EMOTION_CALM,
	EMOTION_SMILE,
	EMOTION_ANGRY,
	EMOTION_CRY,
	EMOTION_SAD,
	EMOTION_SURPRISE,
	EMOTION_DOUBT,
};

class TalkProc
{
public:
	TalkProc()
		: talk_pos(POS_LEFT), speaker_type(SPEAKER_PLAYER), speaker_id(0), emotion(0)
	{
	}

	TalkProc(const TalkProc& rhs)
		: talk_pos(rhs.talk_pos), speaker_type(rhs.speaker_type), 
		speaker_id(rhs.speaker_id), speaker_name(rhs.speaker_name), 
		emotion(rhs.emotion), talk(rhs.talk), options(rhs.options)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t talk_pos;				// 位于界面的左还是右
	int8_t speaker_type;			// 说话人的类型
	int32_t speaker_id;				// 说话人类型为NPC时为NPCID
	std::string speaker_name;		// 说话人的名字
	int8_t emotion;					// 人物立绘表情ID
	std::string talk;				// 对话文字
	OptionVec options;				// 选项列表

	NESTED_DEFINE(talk_pos, speaker_type, speaker_id, speaker_name, emotion, talk, options);
};
typedef std::vector<TalkProc> TalkProcVec;

inline bool Option::CheckDataValidity() const
{
	return type >= OP_TALK_GOTO && type <= OP_SHOW_IMAGE_WITHOUT_PORTRAIT;
}

inline bool TalkProc::CheckDataValidity() const
{
	CHECK_INRANGE(talk_pos, POS_LEFT, POS_RIGHT)
	CHECK_INRANGE(speaker_type, SPEAKER_PLAYER, SPEAKER_NPC)
	CHECK_INRANGE(emotion, EMOTION_CALM, EMOTION_DOUBT)
	CHECK_VEC_VALIDITY(options)
	return true;
}

} // namespace task

#endif // TASK_TALK_PROC_H_
