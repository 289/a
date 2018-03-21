#ifndef SHARED_NET_PACKET_PROTOPACKET_H_
#define SHARED_NET_PACKET_PROTOPACKET_H_

#include <set>

#include "shared/base/singleton.h"
#include "shared/base/types.h"
#include "shared/net/packet/cmd_define.h"
#include "shared/net/packet/base_packet.h"


namespace shared {
namespace net {

///
/// base class for all protocol packet, which be used to communicate with client.
///
class ProtoPacket : public BasePacket
{
public:
	typedef BasePacket::Type Type;

	ProtoPacket(Type id)
		: BasePacket(id)
	{ }
	virtual ~ProtoPacket() { }
};

class DefaultPacket : public ProtoPacket
{
public:
	DefaultPacket(Type id) : ProtoPacket(id)
	{
	}
	virtual ~DefaultPacket()
	{
	}

	virtual DefaultPacket* Clone()
	{
		return new DefaultPacket(*this);
	}
	virtual void Marshal()
	{
	}
	virtual void Unmarshal()
	{
	}
};


///
/// Manage protocol packets, and use the design mode of Prototype
///
class PacketManager : public Singleton<PacketManager>
{
	friend class Singleton<PacketManager>;
public:
	enum ServerType
	{
		COMMON_SERVER = 0,
		CLIENT_TYPE,
		LINK_TYPE,       // linkserver 对部分协议不做marshal,unmarshal直接转发
		GAMESERVER_TYPE,
		MASTER_TYPE,
		GATEWAY_TYPE,
		AUTH_TYPE,
	};

	static inline PacketManager* GetInstance()
	{
		return &(get_mutable_instance());
	}

	static ProtoPacket* CreatePacket(ProtoPacket::Type id);
	static bool InsertPacket(uint16_t type, ProtoPacket* packet);

	static inline bool IsValidType(int32_t type);
	static inline bool IsClientType(int32_t type);
	static inline bool IsLinkType(int32_t type);
	static inline bool IsGameServerType(int32_t type);
	static inline bool IsMasterType(int32_t type);
	static inline bool IsGatewayType(int32_t type);
	static inline bool IsAuthType(int32_t type);

	inline bool CheckServerType(ServerType type, int32_t protoType);
	ProtoPacket* OnCreatePacket(ProtoPacket::Type id);
	bool OnInsertPacket(uint16_t type, ProtoPacket* packet);

protected:
	PacketManager()
	{
	}
	~PacketManager()
	{
	}

private:
	typedef std::map<ProtoPacket::Type, ProtoPacket*> ProtoPacketMap;
	ProtoPacketMap packet_map_;

	typedef std::map<ServerType, std::set<int32_t> > PacketTypeMap;
	PacketTypeMap type_map_;
};


// inline 
#if defined(SERVER_SIDE)
inline bool PacketManager::IsValidType(int32_t type)
{ 
	if ( (type >= C2G_CMD_LOWER_LIMIT && type <= C2G_CMD_UPPER_LIMIT)
			|| (type >= C2M_CMD_LOWER_LIMIT && type <= C2M_CMD_UPPER_LIMIT)
			|| (type >= C2L_CMD_LOWER_LIMIT && type <= C2L_CMD_UPPER_LIMIT)
			|| (type >= C2W_CMD_LOWER_LIMIT && type <= C2W_CMD_UPPER_LIMIT)
			|| (type >= C2A_CMD_LOWER_LIMIT && type <= C2A_CMD_UPPER_LIMIT)
			|| (type >= C2L_DEBUG_CMD_LOWER_LIMIT && type <= C2L_DEBUG_CMD_UPPER_LIMIT)
			|| (type >= C2M_DEBUG_CMD_LOWER_LIMIT && type <= C2M_DEBUG_CMD_UPPER_LIMIT)
			|| (type >= C2G_DEBUG_CMD_LOWER_LIMIT && type <= C2G_DEBUG_CMD_UPPER_LIMIT))
	{
		return true; 
	}

	return false;
}
#elif defined(CLIENT_SIDE)
inline bool PacketManager::IsValidType(int32_t type)
{ 
	if ( (type >= G2C_CMD_LOWER_LIMIT && type <= G2C_CMD_UPPER_LIMIT)
			|| (type >= M2C_CMD_LOWER_LIMIT && type <= M2C_CMD_UPPER_LIMIT)
			|| (type >= L2C_CMD_LOWER_LIMIT && type <= L2C_CMD_UPPER_LIMIT) 
			|| (type >= W2C_CMD_LOWER_LIMIT && type <= W2C_CMD_UPPER_LIMIT)
			|| (type >= A2C_CMD_LOWER_LIMIT && type <= A2C_CMD_UPPER_LIMIT) 
		    || (type >= C2C_CMD_LOWER_LIMIT && type <= C2C_CMD_UPPER_LIMIT) )
	{
		return true; 
	}

	return false;
}
#endif

inline bool PacketManager::CheckServerType(ServerType type, int32_t protoType)
{
	if(type_map_.find(type) == type_map_.end())
	{
		return false;
	}
	return type_map_[type].find(protoType) == type_map_[type].end() ? false : true;
}

inline bool PacketManager::IsClientType(int32_t type)
{
	if( (type >= L2C_CMD_LOWER_LIMIT && type <= L2C_CMD_UPPER_LIMIT)
			|| (type >= W2C_CMD_LOWER_LIMIT && type <= W2C_CMD_UPPER_LIMIT)
			|| (type >= A2C_CMD_LOWER_LIMIT && type <= A2C_CMD_UPPER_LIMIT)
			|| (type >= G2C_CMD_LOWER_LIMIT && type <= G2C_CMD_UPPER_LIMIT)
			|| (type >= M2C_CMD_LOWER_LIMIT && type <= M2C_CMD_UPPER_LIMIT)
			|| (type >= C2C_CMD_LOWER_LIMIT && type <= C2C_CMD_UPPER_LIMIT) )
	{
		return true;
	}
	return false;
}

inline bool PacketManager::IsLinkType(int32_t type)
{
	if ( (type >= C2L_CMD_LOWER_LIMIT && type <= C2L_CMD_UPPER_LIMIT)
	  || (type >= C2L_DEBUG_CMD_LOWER_LIMIT && type <= C2L_DEBUG_CMD_UPPER_LIMIT) )
		return true;

	return false;
}

inline bool PacketManager::IsGatewayType(int32_t type)
{
	return (type >= C2W_CMD_LOWER_LIMIT && type <= C2W_CMD_UPPER_LIMIT) ? true : false;
}

inline bool PacketManager::IsAuthType(int32_t type)
{
	return (type >= C2A_CMD_LOWER_LIMIT && type <= C2A_CMD_UPPER_LIMIT) ? true : false;
}

inline bool PacketManager::IsGameServerType(int32_t type)
{
	if ( (type >= C2G_CMD_LOWER_LIMIT && type <= C2G_CMD_UPPER_LIMIT)
	  || (type >= C2G_DEBUG_CMD_LOWER_LIMIT && type <= C2G_DEBUG_CMD_UPPER_LIMIT) )
		return true;

	return false;
}

inline bool PacketManager::IsMasterType(int32_t type)
{
	if ( (type >= C2M_CMD_LOWER_LIMIT && type <= C2M_CMD_UPPER_LIMIT)
	  || (type >= C2M_DEBUG_CMD_LOWER_LIMIT && type <= C2M_DEBUG_CMD_UPPER_LIMIT) )
		return true;

	return false;
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_PROTOPACKET_H_ 
