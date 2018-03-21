#ifndef GAMED_GS_GLOBAL_MESSAGE_H_
#define GAMED_GS_GLOBAL_MESSAGE_H_

#include <stdint.h>
#include <stddef.h>
#include <memory.h>

#include "shared/base/assertx.h"
#include "gs/player/player_def.h"
#include "game_types.h"


namespace gamed {

#define CHECK_CONTENT_PARAM(msg, param_struct) \
	ASSERT(msg.content_len == sizeof(param_struct));

const int kInvalidMsgType = 0;

/**
 * @brief MSG 
 *    1.MSG是一个copyable的struct，content需要用CopyMsgContent()进行复制
 *    2.MsgQueueList里提供的AddMsg()函数,在进队列之前已经对MSG进行了copy
 */
struct MSG
{
	typedef uint16_t MsgType;

	MsgType          message;       // 消息类型
	struct XID       target;        // 收消息的目标，可能是服务器，玩家，NPC，物品，地图等
	struct XID       source;        // 消息的来源（发送者），可能的id类型同上
	A2DVECTOR        pos;           // 消息发出时的位置，有的消息可能位置并无作用
	int64_t          param;         // 第一个参数，简单消息用这个参数即可
	size_t           content_len;   // 消息的内容长度
	const void*      content;       // 消息的具体内容（不是每个消息都需要）

	MSG()
		: message(kInvalidMsgType),
		  target(),
		  source(),
		  pos(A2DVECTOR()),
		  param(-1),
		  content_len(0),
		  content(NULL)
	{ }

	~MSG()
	{
		message = kInvalidMsgType;
		XID temp;
		target  = temp;
		source  = temp;
		pos     = A2DVECTOR();
		param   = -1;
		content_len = 0;
		content = NULL;
	}
};

/**
 * @brief WORLDMSG
 */
struct WORLDMSG
{
	MapID  world_id;
	MapTag world_tag;
	MSG    msg;
};

#include "message-inl.h"

} // namespace gamed


///
/// MSG define files
///
#include "msg_obj.h"
#include "msg_plane.h"


#endif // GAMED_GS_GLOBAL_MESSAGE_H_
