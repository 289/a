#ifndef COMMON_OBJDATAPOOL_PLAYER_ATTR_UTIL_H
#define COMMON_OBJDATAPOOL_PLAYER_ATTR_UTIL_H


namespace common {

enum
{
	MAIL_OP_NONE =  0x00,
	MAIL_OP_UPDATE_READ = 0x01,
	MAIL_OP_UPDATE_ATTACH = 0x02,
	MAIL_OP_DELETE = 0x04,
};

enum
{
	MAIL_ATTR_READ = 0x001,
	MAIL_ATTR_ATTACH = 0x002,
	MAIL_ATTR_PLAYER = 0x004,
};

inline bool need_update_read(int8_t op)
{
	return op & MAIL_OP_UPDATE_READ;
}

inline bool need_update_attach(int8_t op)
{
	return op & MAIL_OP_UPDATE_ATTACH;
}

inline bool need_delete(int8_t op)
{
	return op & MAIL_OP_DELETE;
}

inline bool readed(int32_t attr)
{
	return attr & MAIL_ATTR_READ;
}

inline bool player_mail(int32_t attr)
{
	return attr & MAIL_ATTR_PLAYER;
}

inline bool sys_mail(int32_t attr)
{
	return !(attr & MAIL_ATTR_PLAYER);
}

inline bool has_attach(int32_t attr)
{
	return attr & MAIL_ATTR_ATTACH;
}

inline void set_sys_mail(int32_t& attr)
{
	attr &= ~MAIL_ATTR_PLAYER;
}

inline void set_player_mail(int32_t& attr)
{
	attr |= MAIL_ATTR_PLAYER;
}

inline void set_delete(int8_t& op)
{
	op |= MAIL_OP_DELETE;
}

inline void takeoff_attach(int8_t& op, int32_t& attr)
{
	attr &= ~MAIL_ATTR_ATTACH;
	op |= MAIL_OP_UPDATE_ATTACH;
}

inline void set_readed(int8_t& op, int32_t& attr)
{
	attr |= MAIL_ATTR_READ;
	op |= MAIL_OP_UPDATE_READ;
}

inline void set_attach(int32_t& attr)
{
	attr |= MAIL_ATTR_ATTACH;
}

} // namespace common

#endif // COMMON_OBJDATAPOOL_PLAYER_ATTR_UTIL_H
