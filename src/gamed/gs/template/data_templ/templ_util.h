#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_UTIL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_UTIL_H_

#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/base_packet.h"


#define INIT_STATIC_DATATEMPLATE(class_name, type) \
	INIT_STATIC_PROTOPACKET(class_name, type)

#define DECLARE_DATATEMPLATE(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, dataTempl::BaseDataTempl, dataTempl::BaseDataTemplManager)

#define DECLARE_ITEM_TEMPLATE(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, dataTempl::ItemDataTempl, dataTempl::BaseDataTemplManager)


// 一行最多支持十六个参数，见packet_util.h
#define MARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

// 超过16个参数的NESTED_DEFINE
#define PACK_NESTED_VALUE(...) \
    shared::detail::make_define(__VA_ARGS__).marshal(buf);

#define UNPACK_NESTED_VALUE(...) \
    shared::detail::make_define(__VA_ARGS__).unmarshal(buf);

#endif // GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_UTIL_H_
