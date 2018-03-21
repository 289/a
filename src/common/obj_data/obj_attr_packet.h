#ifndef COMMON_OBJDATAPOOL_OBJ_ATTR_PACKET_H_
#define COMMON_OBJDATAPOOL_OBJ_ATTR_PACKET_H_

#include "shared/base/singleton.h"

#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/base_packet.h"

#include "common/obj_data/attr_type_def.h"


namespace common {

///
/// object attribute packet, that nested in ObjectData
///
class ObjectAttrPacket : public shared::net::BasePacket
{
public:
	typedef shared::net::BasePacket::Type AttrType;

	ObjectAttrPacket(shared::net::BasePacket::Type id)
		: BasePacket(id)
	{ }

	virtual ~ObjectAttrPacket() { }

	virtual void Release() = 0;
};

///
/// object data manager
///
class ObjAttrPacketManager : public shared::Singleton<ObjAttrPacketManager>
{
	friend class shared::Singleton<ObjAttrPacketManager>;
public:
	static inline ObjAttrPacketManager* GetInstance() {
		return &(get_mutable_instance());
	}

	static ObjectAttrPacket* CreatePacket(ObjectAttrPacket::Type id);
	static bool InsertPacket(uint16_t type, ObjectAttrPacket* packet);
	static bool MarshalPacket(ObjectAttrPacket& packet);
	static ObjectAttrPacket* UnmarshalPacket(uint16_t cmd_type_no, const char* buf, int len);
	static inline bool IsValidType(int32_t type);


protected:
	ObjAttrPacketManager() { }
	~ObjAttrPacketManager() { }

	ObjectAttrPacket* OnCreatePacket(ObjectAttrPacket::Type id);
	bool OnInsertPacket(uint16_t type, ObjectAttrPacket* packet);


private:
	typedef std::map<ObjectAttrPacket::Type, ObjectAttrPacket*> ObjectAttrPacketMap;
	ObjectAttrPacketMap packet_map_;
};

#define s_pObjAttrPacketMan common::ObjAttrPacketManager::GetInstance()

/// 
/// inl
///
inline bool ObjAttrPacketManager::IsValidType(int32_t type)
{
	if ( ((type >= PLAYER_ATTR_TYPE_LOWER_LIMIT) && (type <= PLAYER_ATTR_TYPE_UPPER_LIMIT)) || 
		 ((type >= GLOBAL_DATA_TYPE_LOWER_LIMIT) && (type <= GLOBAL_DATA_TYPE_UPPER_LIMIT)) )
	{
		return true;
	}
	return false;
}

} // namespace common

#endif // COMMON_OBJDATAPOOL_OBJ_ATTR_PACKET_H_
