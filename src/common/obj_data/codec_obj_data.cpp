#include "common/obj_data/codec_obj_data.h"

// total_len是指后面跟的数据的长度，不包含自己的4个字节
/// @code
/// +-----------+------------------+------------+------------------+---------------+-------------+---------
/// | total_len | attr_total_count | attr_type1 | attr_packet_len1 | attr_content1 | attr_type2  | ........
/// |           |                  |            |                  |   (CONTENT)   |             |
/// +-----------+------------------+------------+------------------+---------------+-------------+---------
/// @endcode

namespace common {

using namespace shared;
using namespace shared::net;

int ObjectDataCodec::FillEmptyBuffer(Buffer* buf)
{
	assert(buf->ReadableBytes() == 0);

	uint16_t attr_total_count = obj_attr_map_.size();
	buf->AppendInt16(static_cast<int16_t>(attr_total_count));

	ObjectAttrPtrMap::iterator it = obj_attr_map_.begin();
	for (; it != obj_attr_map_.end(); ++it)
	{
		ObjectAttrPacket* pAttrPacket = it->second;
		pAttrPacket->ClearContent();
		uint16_t attr_type = pAttrPacket->GetType();
		buf->AppendInt16(static_cast<int16_t>(attr_type));
		try
		{
			pAttrPacket->Marshal();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR << "Marshal() failed! Exception FillEmptyBuffer attr_type=" << attr_type;
			return kMarshalError;
		}
		catch (...)
		{
			LOG_ERROR << "Marshal() failed! FillEmptyBuffer attr_type=" << attr_type;
			return kMarshalError;
		}

		int32_t byte_size = pAttrPacket->GetSize();
		buf->AppendInt32(byte_size);
		buf->EnsureWritableBytes(byte_size);
		buf->Append(pAttrPacket->GetContent(), byte_size);
	}

	int32_t total_len = sockets::HostToNetwork32(static_cast<int32_t>(buf->ReadableBytes()));
	buf->Prepend(&total_len, sizeof total_len);

	return kNoError;
}

int32_t AsInt32(const char* buf)
{
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(be32));
	return sockets::NetworkToHost32(be32);
}

uint16_t AsUint16(const char* buf)
{
	int16_t be16 = 0;
	::memcpy(&be16, buf, sizeof(be16));
	return sockets::NetworkToHost16(be16);
}

int ObjectDataCodec::ParseObjectData(const char* pbuf, int32_t len)
{
	if (len <= kMinPacketLen || len > kMaxPacketLen) return kInvalidLength;

	int32_t total_len = AsInt32(pbuf);
	uint16_t attr_total_count = AsUint16(pbuf + kTotalLen);
	assert(len == total_len + kTotalLen);
	const char* pcurbuf = pbuf + kTotalLen + kAttrTotalCount;

	int32_t tmp_total_len = kAttrTotalCount;
	for (size_t i = 0; i < attr_total_count; ++i)
	{
		uint16_t attr_type = AsUint16(pcurbuf);	
		int32_t attr_len   = AsInt32(pcurbuf+kAttrTypeLen);
		tmp_total_len += (attr_len + kAttrTypeLen + kAttrPacketLen);

		ErrorCode errcode = Parse(attr_type, pcurbuf, attr_len);
		if (kNoError != errcode)
		{
			assert(false); // ????????
		}

		pcurbuf += kAttrTypeLen + kAttrPacketLen + attr_len;
	}

	assert(tmp_total_len == total_len);
	return kNoError;
}

ObjectDataCodec::ErrorCode ObjectDataCodec::Parse(int attr_type, const char* buf, int attr_len)
{
	ObjectAttrPtrMap::const_iterator it = obj_attr_map_.find(attr_type);
	if (it != obj_attr_map_.end())
	{
		try
		{
			it->second->ClearContent();
			it->second->AppendBuffer(buf + kAttrTypeLen + kAttrPacketLen, attr_len);
			it->second->Unmarshal();
			it->second->FreeContent();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR << "Unmarshal() failed! Exception ParseObjectData attr_type=" << attr_type;
			return kUnmarshalError;
		}
		catch (...)
		{
			LOG_ERROR << "Unmarshal() failed! ParseObjectData attr_type=" << attr_type;
			return kUnmarshalError;
		}
	}
	else
	{
		LOG_ERROR << "Error: Attr not found - ObjectDataCodec::ParseObjectData() attr_type=" 
			      << attr_type;
		return kUnknownPacketType;
	}

	return kNoError;
}

void ObjectDataCodec::Release()
{
	ObjectAttrPtrMap::iterator it = obj_attr_map_.begin();
	for (; it != obj_attr_map_.end(); ++it)
	{
		it->second->Release();
	}
}

bool ObjectDataCodec::ForeachAttribute(const CodecAttributeCB& callback) const
{
	ObjectAttrPtrMap::const_iterator it = obj_attr_map_.begin();
	for (; it != obj_attr_map_.end(); ++it)
	{
		if (!callback(it->second))
			return false;
	}
	
	return true;
}

} // namespace common
