#ifndef SHARED_NET_PACKET_BYTE_CONVERTER_H_
#define SHARED_NET_PACKET_BYTE_CONVERTER_H_

#include <algorithm>

#include "shared/net/packet/platform_define.h"

namespace shared {
namespace net {

namespace ByteConverter
{
	template<size_t T>
	inline void convert(char* val)
	{
		std::swap(*val, *(val + T - 1));
		convert<T - 2>(val + 1);
	}

	template<> inline void convert<0>(char*) { }
	template<> inline void convert<1>(char*) { }

	template<typename T>
	inline void apply(T* val)
	{
		convert<sizeof(T)>((char*)(val));
	}
} // namespace ByteConverter

#if SHARED_ENDIAN == SHARED_BIGENDIAN
template<typename T> inline void EndianConvert(T& val) { ByteConverter::apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }
#else
template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { ByteConverter::apply<T>(&val); }
#endif

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(unsigned char&) { }
inline void EndianConvert(char&)  { }
inline void EndianConvertReverse(unsigned char&) { }
inline void EndianConvertReverse(char&) { }

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_BYTE_CONVERTER_H_ 
