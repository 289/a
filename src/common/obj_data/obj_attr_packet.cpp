#include "common/obj_data/obj_attr_packet.h"

#include "shared/base/assertx.h"
#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"

// object attr range
SHARED_STATIC_ASSERT(OBJECT_ATTR_TYPE_UPPER_LIMIT >= OBJECT_ATTR_TYPE_LOWER_LIMIT);

// player attr
SHARED_STATIC_ASSERT(PLAYER_ATTR_TYPE_UPPER_LIMIT >= PLAYER_ATTR_TYPE_LOWER_LIMIT);
SHARED_STATIC_ASSERT(PLAYER_ATTR_TYPE_LOWER_LIMIT > OBJECT_ATTR_TYPE_LOWER_LIMIT);
SHARED_STATIC_ASSERT(PLAYER_ATTR_TYPE_UPPER_LIMIT < GLOBAL_DATA_TYPE_LOWER_LIMIT);

// global data
SHARED_STATIC_ASSERT(GLOBAL_DATA_TYPE_UPPER_LIMIT >= GLOBAL_DATA_TYPE_LOWER_LIMIT);
SHARED_STATIC_ASSERT(GLOBAL_DATA_TYPE_LOWER_LIMIT > PLAYER_ATTR_TYPE_UPPER_LIMIT);
SHARED_STATIC_ASSERT(GLOBAL_DATA_TYPE_UPPER_LIMIT < OBJECT_ATTR_TYPE_UPPER_LIMIT);


namespace common {
	
using namespace shared;
using namespace shared::net;

bool ObjAttrPacketManager::InsertPacket(uint16_t type, ObjectAttrPacket* packet)
{
	return GetInstance()->OnInsertPacket(type, packet);
}

ObjectAttrPacket* ObjAttrPacketManager::CreatePacket(ObjectAttrPacket::Type id)
{
	return GetInstance()->OnCreatePacket(id);
}

bool ObjAttrPacketManager::MarshalPacket(ObjectAttrPacket& packet)
{
	try
	{
		packet.ClearContent();
		packet.Marshal();
	}
	catch (const Exception& ex)
	{
		LOG_ERROR << "Marshal() failed! Exception packet_type:" << packet.GetType();
		return false;
	}
	catch (...)
	{
		LOG_ERROR << "Marshal() failed! packet_type:" << packet.GetType();
		return false;
	}

	return true;
}

ObjectAttrPacket* ObjAttrPacketManager::UnmarshalPacket(uint16_t cmd_type_no, const char* buf, int len)
{
	ObjectAttrPacket* packet = NULL;
	if (IsValidType(static_cast<int32_t>(cmd_type_no)))
	{
		packet = CreatePacket(cmd_type_no);
		if (packet)
		{
			try
			{
				packet->AppendBuffer(buf, len);
				packet->Unmarshal();
			}
			catch (const Exception& ex)
			{
				LOG_ERROR << "Unmarshal() failed! Exception packet_type:" << cmd_type_no;
				SAFE_DELETE(packet);
				return NULL;
			}
			catch (...)
			{
				LOG_ERROR << "Unmarshal() failed! packet_type:" << cmd_type_no;
				SAFE_DELETE(packet);
				return NULL;
			}
		}
		else
		{
			LOG_ERROR << "CreatePacket() error! packet_type:" << cmd_type_no;
			return NULL;
		}
	}
	else
	{
		LOG_ERROR << "ObjectAttrPacket type is invalid! packet_type:" << cmd_type_no;
		return NULL;
	}

	return packet;
}

bool ObjAttrPacketManager::OnInsertPacket(uint16_t type, ObjectAttrPacket* packet)
{
	if (!packet_map_.insert(std::make_pair(type, packet)).second)
	{
		assert(false);
		return false;
	}

	return true;
}

ObjectAttrPacket* ObjAttrPacketManager::OnCreatePacket(ObjectAttrPacket::Type id)
{
	ObjectAttrPacketMap::iterator it = packet_map_.find(id);	
	if (packet_map_.end() == it) return NULL;

	return dynamic_cast<ObjectAttrPacket*>(it->second->Clone());
}

} // namespace common
