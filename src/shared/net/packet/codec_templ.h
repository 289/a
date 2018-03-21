#ifndef SHARED_NET_PACKET_CODEC_TEMPL_H_
#define SHARED_NET_PACKET_CODEC_TEMPL_H_

#include <stdint.h>
#include <zlib.h> // adler32

#include "shared/net/buffer.h"

#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"


namespace shared {
namespace net {

/**
 * total_len 表示后面跟的长度，并不包含自身的4个字节
 * @code for the whole elementdata file
 * +-----------+---------+----------------------+---------------+---------------+----------+-----------+
 * | total_len | version | template_total_count | templ_packet1 | templ_packet2 | ........ | checksum  |
 * |           |         |                      |   (CONTENT)   |   (CONTENT)   |          | (adler32) | 
 * +-----------+---------+----------------------+---------------+---------------+----------+-----------+
 * @endcode
 *
 * "templ_packet" define as follow:
 * len 表示后面跟的长度，并不包含自身的4个字节
 * @code for single packet (templ_packet - see above)
 * +------------+-------------------+----------------------+
 * | packet_len | templ_packet_type | templ_packet_content |
 * |            |                   |      (CONTENT)       |
 * +------------+-------------------+----------------------+
 * @endcode
 */

enum TPCErrorCode
{
	TPC_SUCCESS = 0,
	TPC_INVALID_LENGTH,
	TPC_CHECK_SUM_ERROR,
	TPC_VERSION_ERROR,
	TPC_PARSE_ERROR,
	// 5
	TPC_TEMPL_PACKET_TYPE_ERROR,
	TPC_PACKET_LEN_ERROR,
	TPC_PACKET_TYPE_NOT_FOUND,
	TPC_PACKET_UNMARSHAL_ERROR,
	TPC_PACKET_MARSHAL_ERROR,
};

template <typename T>
class TemplPacketCodec
{
public:
	const static int kTotalLenHeaderLen     = sizeof(int32_t);   
	const static int kVersionLen            = sizeof(int32_t);
	const static int kTotalCountLen         = sizeof(int32_t); 
	const static int kCheckSumLen           = sizeof(int32_t);
	const static int kMinElementDataFileLen = kVersionLen + kTotalCountLen + kCheckSumLen;
	const static int kPacketLenHeaderLen    = sizeof(int32_t);
	const static int kPacketTypeLen         = sizeof(uint16_t);
	
	typedef T* (*CreatePacketCB)(typename T::Type id);
	typedef bool (*CheckPacketTypeCB)(int32_t type);

	TemplPacketCodec(int32_t version, CreatePacketCB create_cb, CheckPacketTypeCB check_cb);
	virtual ~TemplPacketCodec();

	TPCErrorCode AssemblingEmptyBuffer(Buffer* buf, std::vector<T*>& vecTemplPtr);
	TPCErrorCode ParseBuffer(const Buffer* buf, std::vector<T*>& vecTemplPtr);


protected:
	TPCErrorCode Parse(const char* buf, 
	   		           int32_t count, 
			           int32_t len, 
			           std::vector<T*>& vecTemplPtr);

	void SinglePacketFillEmptyBuffer(Buffer* buf, const T& packet);

	inline int32_t AsInt32(const char* buf);
	inline int16_t AsInt16(const char* buf);


private:
	TemplPacketCodec();
	T* CreateTemplPacket(typename T::Type typenum);

	int32_t           templ_version_;
	CreatePacketCB    create_packet_cb_;
	CheckPacketTypeCB check_type_cb_;
};

///
/// member functions
///
template <typename T>
TemplPacketCodec<T>::TemplPacketCodec()
	: templ_version_(-1),
	  create_packet_cb_(NULL),
	  check_type_cb_(NULL)
{
}

template <typename T>
TemplPacketCodec<T>::TemplPacketCodec(int32_t version, CreatePacketCB create_cb, CheckPacketTypeCB check_cb)
	: templ_version_(version),
	  create_packet_cb_(create_cb),
	  check_type_cb_(check_cb)
{
}

template <typename T>
TemplPacketCodec<T>::~TemplPacketCodec()
{
}

template <typename T>
T* TemplPacketCodec<T>::CreateTemplPacket(typename T::Type typenum)
{
	T* packet = NULL;
	packet = create_packet_cb_(typenum);
	return packet;
}

template <typename T>
void TemplPacketCodec<T>::SinglePacketFillEmptyBuffer(Buffer* buf, const T& packet)
{
	assert(buf->ReadableBytes() == 0);

	uint16_t typenum = packet.GetType();
	buf->AppendInt16(static_cast<int16_t>(typenum));

	uint32_t byte_size = packet.GetSize();
	if (byte_size != 0)
	{
		buf->EnsureWritableBytes(byte_size);
		buf->Append(packet.GetContent(), byte_size);
	}

	assert(buf->ReadableBytes() == sizeof typenum + byte_size);
	int32_t len = sockets::HostToNetwork32(static_cast<int32_t>(buf->ReadableBytes()));
	buf->Prepend(&len, sizeof len);
}

template <typename T>
TPCErrorCode TemplPacketCodec<T>::AssemblingEmptyBuffer(Buffer* buf, std::vector<T*>& vecTemplPtr)
{
	assert(buf->ReadableBytes() == 0);

	int32_t version = templ_version_;
	buf->AppendInt32(version);

	int32_t templ_total_count = vecTemplPtr.size();
	buf->AppendInt32(templ_total_count);

	Buffer tempbuf;
	int32_t content_total_len = 0;
	for (uint32_t i = 0; i < vecTemplPtr.size(); ++i)
	{
		try
		{
			tempbuf.RetrieveAll();
			T* packet = vecTemplPtr[i];
			packet->Marshal();
			SinglePacketFillEmptyBuffer(&tempbuf, *packet);
			content_total_len += tempbuf.ReadableBytes();
			buf->Append(tempbuf.peek(), tempbuf.ReadableBytes());
		}
		catch (...)
		{
			return TPC_PACKET_MARSHAL_ERROR;
		}
	}

	int32_t checksum = static_cast<int32_t>(::adler32(1,
										              reinterpret_cast<const Bytef*>(buf->peek()),
												      static_cast<int>(buf->ReadableBytes())));
	buf->AppendInt32(checksum);
	assert(buf->ReadableBytes() == sizeof version + sizeof templ_total_count + content_total_len + sizeof checksum);
	int32_t total_len = sockets::HostToNetwork32(static_cast<int32_t>(buf->ReadableBytes()));
	buf->Prepend(&total_len, sizeof total_len);

	return TPC_SUCCESS;
}

template <typename T>
inline int32_t TemplPacketCodec<T>::AsInt32(const char* buf)
{
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(be32));
	return sockets::NetworkToHost32(be32);
}

template <typename T>
inline int16_t TemplPacketCodec<T>::AsInt16(const char* buf)
{
	int16_t be16 = 0;
	::memcpy(&be16, buf, sizeof(be16));
	return sockets::NetworkToHost16(be16);
}

template <typename T>
TPCErrorCode TemplPacketCodec<T>::ParseBuffer(const Buffer* buf, 
		                                      std::vector<T*>& vecTemplPtr)
{
	assert(vecTemplPtr.size() == 0);

	if (buf->ReadableBytes() < kMinElementDataFileLen + kTotalLenHeaderLen) 
		return TPC_INVALID_LENGTH;

	const int32_t len = buf->PeekInt32();
	if (len < kMinElementDataFileLen)
		return TPC_INVALID_LENGTH;

	// check sum
	const char* pbufContent = buf->peek() + kTotalLenHeaderLen;
	int32_t expectedCheckSum = AsInt32(pbufContent + len - kCheckSumLen);
	int32_t checkSum = static_cast<int32_t>(::adler32(1,
										              reinterpret_cast<const Bytef*>(pbufContent),
									                  static_cast<int>(len - kCheckSumLen)));
	if (checkSum == expectedCheckSum)
	{
		int32_t version = AsInt32(pbufContent);
		if (version == templ_version_)
		{
			int32_t templ_total_count = AsInt32(pbufContent+kVersionLen);
			TPCErrorCode result = Parse(pbufContent+kVersionLen+kTotalCountLen,
					                    templ_total_count,
						   		        len - kVersionLen - kTotalCountLen - kCheckSumLen,
								        vecTemplPtr);
			if (TPC_SUCCESS != result)
			{
				for (uint32_t i = 0; i < vecTemplPtr.size(); ++i)
				{
					SAFE_DELETE(vecTemplPtr[i]);
				}
				vecTemplPtr.clear();
				return TPC_PARSE_ERROR;
			}
		}
		else
		{
			return TPC_VERSION_ERROR;
		}
	}
	else
	{
		return TPC_CHECK_SUM_ERROR;
	}

	return TPC_SUCCESS;
}

template <typename T>
TPCErrorCode TemplPacketCodec<T>::Parse(const char* buf, 
                                        int32_t count, 
                                        int32_t len, 
                                        std::vector<T*>& vecTemplPtr)
{
	int32_t calc_len     = len;
	int32_t check_count  = 0;
	const char* ptempbuf = buf;
	while (calc_len > 0)
	{
		int32_t packet_len = AsInt32(ptempbuf);
		if (packet_len < kPacketTypeLen)
			return TPC_PACKET_LEN_ERROR;

		T* packet = NULL;
		uint16_t typenum   = static_cast<uint16_t>(AsInt16(ptempbuf+kPacketLenHeaderLen));
		uint32_t templ_id  = 0;
		if (check_type_cb_(typenum))
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
                    packet->FreeContent();
				}
				catch (...)
				{
					LOG_ERROR << "Loading template packet:" << packet->GetName() << "failed! templ_type= "<< typenum << "templ_id=" << templ_id;
					SAFE_DELETE(packet);
					return TPC_PACKET_UNMARSHAL_ERROR; 
				}
			}
			else
			{
				LOG_ERROR << "Loading template packet failed!" << " Reason: packet_type=" << typenum << " not found!";
				return TPC_PACKET_TYPE_NOT_FOUND;
			}
		}
		else
		{
			return TPC_TEMPL_PACKET_TYPE_ERROR;
		}

		vecTemplPtr.push_back(packet);

		// ptr shift
		ptempbuf += kPacketLenHeaderLen + packet_len;
		calc_len -= (kPacketLenHeaderLen + packet_len);
		++check_count;
	}

	assert(check_count == count);
	return TPC_SUCCESS;
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_CODEC_TEMPL_H_
