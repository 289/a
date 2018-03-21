#ifndef COMMON_OBJDATAPOOL_CODEC_OBJ_DATA_H_
#define COMMON_OBJDATAPOOL_CODEC_OBJ_DATA_H_

#include <stdio.h>

#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"
#include "shared/net/buffer.h"
#include "shared/net/packet/base_packet.h"
#include "shared/logsys/logging.h"

#include "common/obj_data/obj_attr_packet.h"


namespace common {

class ObjectData;

class ObjectDataCodec : shared::noncopyable
{
public:
	typedef shared::bind::Callback<bool (ObjectAttrPacket*)> CodecAttributeCB;

	const static int kTotalLen       = sizeof(int32_t);
	const static int kAttrTotalCount = sizeof(uint16_t);
	const static int kAttrTypeLen    = sizeof(uint16_t);
	const static int kAttrPacketLen  = sizeof(int32_t);
	const static int kMinPacketLen   = kTotalLen + kAttrTotalCount + kAttrTypeLen + kAttrPacketLen;
	const static int kMaxPacketLen   = 2*1024*1024; // 2M just like: shared/net/packet/codec_packet.h

	enum ErrorCode
	{
		kNoError = 0,
		kInvalidparam,
		kInvalidLength,
		kUnknownPacketType,
		kUnmarshalError,
		kMarshalError,
	};  

	int    ParseObjectData(const char* pbuf, int32_t len);
    int    FillEmptyBuffer(shared::net::Buffer* buf);
	bool   ForeachAttribute(const CodecAttributeCB& callback) const;

	void   Release();

	template<typename T>
	void   RegisterAttr(ObjectAttrPacket* attr_ptr)
	{
		ObjectAttrPtrMap::const_iterator it = obj_attr_map_.find(T::TypeNumber());
		if (it != obj_attr_map_.end())
		{
			LOG_ERROR << "Error: repeated registration - ObjectDataCodec";
			assert(false);
			return;
		}
		obj_attr_map_[T::TypeNumber()] = attr_ptr;
	}


private:
	ErrorCode Parse(int attr_type, const char* buf, int attr_len);


private:
	typedef std::map<const shared::net::BasePacket::Type, ObjectAttrPacket*> ObjectAttrPtrMap;
	ObjectAttrPtrMap obj_attr_map_;
};

} // namespace common

#endif // COMMON_OBJDATAPOOL_CODEC_OBJ_DATA_H_
