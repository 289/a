#ifndef COMMON_OBJDATAPOOL_OBJ_PACKET_UTIL_H_
#define COMMON_OBJDATAPOOL_OBJ_PACKET_UTIL_H_

#include "shared/net/packet/packet_util.h"
#include "common/obj_data/obj_attr_packet.h"

///
/// init a objectdata_packet: must in cpp file
///
#define INIT_STAITC_OBJECTATTR(class_name, type) \
	INIT_STATIC_PROTOPACKET(class_name, type)

///
/// declare a objectdata_packet
///
#define DECLARE_OBJECTATTR(class_name, type) \
	DECLARE_PACKET_COMMON_TEMPLATE(class_name, type, common::ObjectAttrPacket)

///
/// declare global data
///
#define DECLARE_GLOBALDATA(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, common::ObjectAttrPacket, common::ObjAttrPacketManager);

///
/// implement marshal/unmarshal code
///
#define OBJECTATTR_DEFINE(...) \
    virtual void Marshal() \
    { \
		shared::detail::make_define(__VA_ARGS__).marshal(buf_); \
	} \
    virtual void Unmarshal()\
    { \
		shared::detail::make_define(__VA_ARGS__).unmarshal(buf_); \
	}

#define MARSHAL_ATTR(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_ATTR(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);



namespace common {
namespace detail {

// integer
//
inline void type_release(uint8_t& var) {
	var = 0;
}

inline void type_release(uint16_t& var) {
	var = 0;
}

inline void type_release(uint32_t& var) {
	var = 0;
}

inline void type_release(uint64_t& var) {
	var = 0;
}

inline void type_release(int8_t& var) {
	var = -1;
}

inline void type_release(int16_t& var) {
	var = -1;
}

inline void type_release(int32_t& var) {
	var = -1;
}

inline void type_release(int64_t& var) {
	var = -1;
}

inline void type_release(float& var) {
	var = 0.0f;
}

inline void type_release(double& var) {
	var = 0.0f;
}

inline void type_release(bool& var) {
	var = false;
}

inline void type_release(std::string& str) {
	str.clear();
}

//inline void type_release(shared::net::FixedArray* array) {
	//array.clear();
//}

//inline void type_release(std::vector)

} // namespace detail
} // namespace common
	
#endif // COMMON_OBJDATAPOOL_OBJ_PACKET_UTIL_H_
