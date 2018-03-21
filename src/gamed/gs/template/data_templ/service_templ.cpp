#include "service_templ.h"

#include "shared/base/base_define.h"
#include "shared/net/endian_ex.h"
#include "shared/net/buffer.h"
#include "shared/logsys/logging.h"


namespace serviceDef {

// 0 SRV_DEF_INVALID
INIT_STAITC_SERVICE_TEMPL(SimpleServiceTempl, SRV_DEF_SIMPLE_SERVICE);
INIT_STAITC_SERVICE_TEMPL(DeliverTaskSrvTempl, SRV_DEF_DELIVER_TASK);
INIT_STAITC_SERVICE_TEMPL(RecycleTaskSrvTempl, SRV_DEF_RECYCLE_TASK);
INIT_STAITC_SERVICE_TEMPL(ShopSrvTempl, SRV_DEF_SHOP);

// 5
INIT_STAITC_SERVICE_TEMPL(StorageTaskSrvTempl, SRV_DEF_STORAGE_TASK);
INIT_STAITC_SERVICE_TEMPL(EnhanceSrvTempl, SRV_DEF_ENHANCE);


///
/// Anonymous namespace
///
namespace 
{
	const static int kTotalLenHeaderLen     = sizeof(int32_t);   
	const static int kTotalCountLen         = sizeof(int32_t); 
	const static int kPacketLenHeaderLen    = sizeof(int32_t);
	const static int kPacketTypeLen         = sizeof(uint16_t);
	const static int kMinElementDataFileLen = kPacketLenHeaderLen + kPacketTypeLen;

	int32_t asInt32(const char* buf)
	{
		int32_t be32 = 0;
		::memcpy(&be32, buf, sizeof(be32));
		return sockets::NetworkToHost32(be32);
	}

	int16_t asInt16(const char* buf)
	{
		int16_t be16 = 0;
		::memcpy(&be16, buf, sizeof(be16));
		return sockets::NetworkToHost16(be16);
	}

	ServiceTempl* CreateTemplPacket(ServiceTempl::SrvType typenum)
	{
		ServiceTempl* packet = NULL;
		packet = ServiceTemplManager::CreatePacket(typenum);
		return packet;
	}

	bool Parse(const char* buf, int32_t count, int32_t len, std::vector<ServiceTempl*>& vecTemplPtr)
	{
		int32_t calc_len     = len;
		int32_t check_count  = 0;
		const char* ptempbuf = buf;
		while (calc_len > 0)
		{
			int32_t packet_len = asInt32(ptempbuf);
			if (packet_len < kPacketTypeLen)
				return false;

			ServiceTempl* packet = NULL;
			uint16_t typenum     = static_cast<uint16_t>(asInt16(ptempbuf+kPacketLenHeaderLen));
			uint32_t templ_id    = 0;
			if (ServiceTemplManager::IsValidType(typenum))
			{
				packet = CreateTemplPacket(typenum);
				if (packet)
				{
					try
					{
						const char* data = ptempbuf + kPacketLenHeaderLen + kPacketTypeLen;
						int32_t dataLen  = packet_len - kPacketTypeLen;
						templ_id = *((int32_t*)data);
						packet->AppendBuffer(data, dataLen);
						packet->Unmarshal();
					}
					catch (...)
					{
						LOG_ERROR << "加载ServiceTempl失败，服务模板类型= "<< typenum;
						SAFE_DELETE(packet);
						return false; 
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			vecTemplPtr.push_back(packet);

			// ptr shift
			ptempbuf += kPacketLenHeaderLen + packet_len;
			calc_len -= (kPacketLenHeaderLen + packet_len);
			++check_count;
		}

		if (check_count != count)
		{
			return false;
		}
		return true;
	}

	void SinglePacketFillEmptyBuffer(shared::net::Buffer* buf, const ServiceTempl& packet)
	{
		assert(buf->ReadableBytes() == 0);

		uint16_t typenum = packet.GetType();
		buf->AppendInt16(static_cast<int16_t>(typenum));

		size_t byte_size = packet.GetSize();
		if (byte_size != 0)
		{
			buf->EnsureWritableBytes(byte_size);
			buf->Append(packet.GetContent(), byte_size);
		}

		assert(buf->ReadableBytes() == sizeof typenum + byte_size);
		int32_t len = sockets::HostToNetwork32(static_cast<int32_t>(buf->ReadableBytes()));
		buf->Prepend(&len, sizeof len);
	}

} // Anonymous

/// total_len 表示后面跟的长度，并不包含自身的4个字节
/// @code for the whole elementdata file
/// +-----------+----------------------+---------------+---------------+----------+
/// | total_len | template_total_count | templ_packet1 | templ_packet2 | ........ |
/// |           |                      |   (CONTENT)   |   (CONTENT)   |          |
/// +-----------+----------------------+---------------+---------------+----------+
/// @endcode
///
/// "ServiceTempl: templ_packet" defined as follow:
/// len 表示后面跟的长度，并不包含自身的4个字节
/// @code for single packet
/// +------------+-------------------+----------------------+
/// | packet_len | templ_packet_type | templ_packet_content |
/// |            |                   |      (CONTENT)       |
/// +------------+-------------------+----------------------+
/// @endcode
bool FillEmptyBuffer(std::string& buf, const std::vector<ServiceTempl*>& vecTemplPtr)
{
	// 如果是空则返回true
	if (vecTemplPtr.empty())
	{
		buf.clear();
		return true;
	}

	for (size_t i = 0; i < vecTemplPtr.size(); ++i)
	{
		if (!vecTemplPtr[i]->CheckDataValidity())
			return false;
	}

	assert(buf.size() == 0);

	Buffer total_buf;
	int32_t templ_total_count = vecTemplPtr.size();
	total_buf.AppendInt32(templ_total_count);

	Buffer tempbuf;
	int32_t content_total_len = 0;
	for (size_t i = 0; i < vecTemplPtr.size(); ++i)
	{
		try
		{
			tempbuf.RetrieveAll();
			ServiceTempl* packet = vecTemplPtr[i];
			packet->Marshal();
			SinglePacketFillEmptyBuffer(&tempbuf, *packet);
			content_total_len += tempbuf.ReadableBytes();
			total_buf.Append(tempbuf.peek(), tempbuf.ReadableBytes());
		}
		catch (...)
		{
			return false;
		}
	}

	assert(total_buf.ReadableBytes() == sizeof templ_total_count + content_total_len);
	int32_t total_len = sockets::HostToNetwork32(static_cast<int32_t>(total_buf.ReadableBytes()));
	total_buf.Prepend(&total_len, sizeof total_len);

	buf.assign(total_buf.peek(), total_buf.ReadableBytes());
	return true;
}

bool ParseBuffer(const std::string& buf, std::vector<ServiceTempl*>& vecTemplPtr)
{
	// 如果是空则返回true
	if (buf.empty())
	{
		vecTemplPtr.clear();
		return true;
	}

	if (vecTemplPtr.size() != 0)
		return false;

	// 无服务，返回true
	if (buf.size() == 0)
		return true;

	const int32_t total_len = asInt32(buf.c_str());
	// buf长度无效,至少有一个服务的packet
	if ( sizeof(total_len) != kTotalLenHeaderLen 
	  || total_len + sizeof(total_len) != buf.size()
	  || total_len < (kTotalLenHeaderLen + kTotalCountLen + kMinElementDataFileLen))
		return false;

	// 检查个数
	const int32_t count = asInt32(buf.c_str() + kTotalLenHeaderLen);
	if (sizeof(count) != kTotalCountLen || count <= 0)
		return false;

	// parse each packet_content
	const int32_t len    = total_len - kTotalCountLen;
	const char* data_ptr = buf.c_str() + sizeof(total_len) + sizeof(count);
	if (!Parse(data_ptr, count, len, vecTemplPtr))
	{
		LOG_ERROR << "service content string Parse() error!";
		return false;
	}

	// 一个数据出错则整个数组删除
	for (size_t i = 0; i < vecTemplPtr.size(); ++i)
	{
		if (!vecTemplPtr[i]->CheckDataValidity())
		{
			LOG_ERROR << "service content string CheckDataValidity() failure! type:" 
				<< vecTemplPtr[i]->GetType();
			return false;
		}
	}
	return true;
}

///
/// ServiceTemplManager
///
bool ServiceTemplManager::InsertPacket(uint16_t type, ServiceTempl* packet)
{
	return ServiceTemplManager::GetInstance()->OnInsertPacket(type, packet);
}

ServiceTempl* ServiceTemplManager::CreatePacket(ServiceTempl::SrvType id)
{
	return ServiceTemplManager::GetInstance()->OnCreatePacket(id);
}

bool ServiceTemplManager::IsValidType(int32_t type)
{
	if (type > SRV_DEF_INVALID && type < SRV_DEF_MAX)
		return true;
	return false;
}

bool ServiceTemplManager::OnInsertPacket(uint16_t type, ServiceTempl* packet)
{
	if (!packet_map_.insert(std::make_pair(type, packet)).second)
	{
		assert(false); 
		return false;
	}
	return true;
}

ServiceTempl* ServiceTemplManager::OnCreatePacket(ServiceTempl::SrvType id)
{
	ServiceTemplMap::iterator it = packet_map_.find(id);
	if (packet_map_.end() == it) return NULL;

	return static_cast<ServiceTempl*>(it->second->Clone());
}

} // namespace serviceDef
