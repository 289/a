#ifndef GAMED_GS_GLOBAL_MESSAGE_INL_H_
#define GAMED_GS_GLOBAL_MESSAGE_INL_H_


///
/// inline func
///
inline void* CopyMsgContent(const MSG& msg)
{
	uint8_t* buf     = NULL;
	size_t length = msg.content_len;
	if (length)
	{
		buf = new uint8_t[length];
		memcpy(reinterpret_cast<uint8_t*>(buf), msg.content, length);
	}

	return buf;
}

inline void CopyMessage(MSG& dest_msg, const MSG& src_msg)
{
	dest_msg         = src_msg;
	dest_msg.content = CopyMsgContent(src_msg);
}

inline void FreeMessage(MSG* pMsg)
{
	if (!pMsg->content_len) return;
	//ASSERT(pMsg->content == reinterpret_cast<uint8_t*>(pMsg) + sizeof(MSG));
	delete [] (uint8_t*)(const_cast<void*>(pMsg->content));
}

inline void FreeMessage(WORLDMSG* pMsg)
{
	FreeMessage(&pMsg->msg);
}

inline void BuildMessage(MSG& msg, 
		                 int type, 
						 const XID& target, 
						 const XID& source,
						 int64_t param = 0, 
						 const void* content = NULL,
						 size_t content_len = 0,
						 A2DVECTOR pos = A2DVECTOR())
{
	msg.message = type;
	msg.target  = target;
	msg.source  = source;
	msg.pos     = pos;
	msg.param   = param;
	msg.content_len = content_len;
	msg.content = content;
}

template <typename T>
inline void MsgContentMarshal(T& content, shared::net::ByteBuffer& buf)
{
	try
	{
		buf << content;
	}
	catch (...)
	{
		ASSERT(false);
	}
}

template <typename T>
inline void MsgContentUnmarshal(const MSG& msg, T& ret)
{
	ASSERT(msg.content_len > 0);
	shared::net::ByteBuffer tmpbuf;
	tmpbuf.append(static_cast<const char*>(msg.content), msg.content_len);
	try
	{
		tmpbuf >> ret;
	}
	catch (...)
	{
		ASSERT(false);
	}
}

#endif // GAMED_GS_GLOBAL_MESSAGE_INL_H_

