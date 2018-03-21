#ifndef GAMED_GS_TEMPLATE_MAPDATA_MAP_UTIL_H_
#define GAMED_GS_TEMPLATE_MAPDATA_MAP_UTIL_H_

#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/base_packet.h"


#define INIT_STAITC_MAPDATA(class_name, type) \
	INIT_STATIC_PROTOPACKET(class_name, type)

#define DECLARE_MAPDATA(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, mapDataSvr::BaseMapData, mapDataSvr::BaseMapDataManager)


// 一行最多支持十六个参数，见packet_util.h
#define MARSHAL_MAPDATA(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_MAPDATA(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

#endif // GAMED_GS_TEMPLATE_MAPDATA_MAP_UTIL_H_
