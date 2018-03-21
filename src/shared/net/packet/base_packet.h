#ifndef SHARED_NET_PACKET_BASEPACKET_H_
#define SHARED_NET_PACKET_BASEPACKET_H_

#include "shared/net/packet/bytebuffer.h"


namespace shared {
namespace net {

///
/// base class for all packet, which be used to marshal or unmarshal form stream
///
class BasePacket
{
public:
	typedef uint16_t Type;

	BasePacket(Type id)
		: typeid_(id)
	{ }
	virtual ~BasePacket() { }

	virtual BasePacket* Clone()  = 0;
	virtual void Marshal()       = 0;
	virtual void Unmarshal()     = 0;

	virtual bool IsValid() { return true; }
	virtual void Release() { } // Call this function when class construct

	inline std::string GetName() const { return TypeName(); }
	inline uint16_t GetType() const { return static_cast<uint16_t>(typeid_); }
	inline size_t   GetSize() const { return buf_.size(); }
	inline const uint8_t* GetContent() const { return buf_.contents(); }
	inline void     ClearContent() { buf_.clear(); } // only clean buffer, rather than free memory
	inline void     FreeContent() { buf_.free(); }   // truely free memory

	void AppendBuffer(const char* srcbuf, size_t cnt) { buf_.append(srcbuf, cnt); }

	// This function implement in each derived class. by DECLARE_PROTOPACKET() 
	inline static uint16_t TypeNumber() { return -1; }
	inline static std::string TypeName() { return std::string(); }


protected:
	Type        typeid_;
	ByteBuffer  buf_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_BASEPACKET_H_
