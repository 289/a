#include <stdlib.h>
#include <string.h>
#include "combat_msg.h"

namespace  combat
{

void DupeMessage(MSG& dest_msg, const MSG& msg)
{
	dest_msg = msg;
	if (msg.content_len > 0)
	{
		void* buf = malloc(msg.content_len);
		memcpy(buf, msg.content, msg.content_len);
		dest_msg.content = buf;
	}
}

void FreeMessage(MSG& msg)
{
	if (msg.content != NULL)
	{
		free((void*)(msg.content));
	}
}

void BuildMessage(MSG& msg, int message, const XID& target, const XID& source, int64_t param, const void* buf, int len)
{
	msg.message = message;
	msg.target = target;
	msg.source = source;
	msg.param = param;
	msg.content_len = len;
	msg.content = buf;
}

}// namespace combat
