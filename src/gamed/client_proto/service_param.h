#ifndef GAMED_CLIENT_PROTO_SERVICE_PARAM_H_
#define GAMED_CLIENT_PROTO_SERVICE_PARAM_H_

#include "shared/net/packet/packet_util.h"


namespace serviceDef {

typedef int32_t TaskID;

///
/// 功能函数
///
template <typename T>
void PackSrvParam(const T& param, std::string& out)
{
	shared::net::ByteBuffer tmpbuf;
	try
	{
		tmpbuf << const_cast<T&>(param);
	}
	catch (...)
	{
		assert(false);
	}
	out.assign((const char*)tmpbuf.contents(), tmpbuf.size());
}

template <typename T>
bool UnpackSrvParam(const std::string& in, T& param)
{
	shared::net::ByteBuffer tmpbuf;
	try
	{
		tmpbuf.append(in.c_str(), in.size());
		tmpbuf >> param;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

template <typename T>
bool UnpackSrvParam(const void* buf, size_t size, T& param)
{
	shared::net::ByteBuffer tmpbuf;
	try
	{
		tmpbuf.append((uint8_t*)buf, size);
		tmpbuf >> param;
	}
	catch (...)
	{
		return false;
	}
	return true;
}


// 所有struct都必须向1个字节对齐
//#pragma pack(1)

/**
 * @brief 
 *    以下定义的所有struct都必须是copyable的
 *    struct写上对应服务器的服务号作为注释，如SRV_DEF_DELIVER_TASK
 *    服务号的enum定义在data_templ/service_templ.h
 */

// SRV_DEF_DELIVER_TASK
struct deliver_task
{
	TaskID task_id;
	NESTED_DEFINE(task_id);
};

// SRV_DEF_RECYCLE_TASK
struct recycle_task
{
	TaskID  task_id;
	int16_t choice;
	NESTED_DEFINE(task_id, choice);
};

// SRV_DEF_SHOP
struct shop_shopping
{
	int32_t goods_id;      // 商品ID(唯一标识本商品)，客户端设置
	int32_t goods_count;   // 购买几个商品，客户端设置
	int32_t goods_remains; // 购买后还有多少商品可售，由服务器设置
	int32_t goods_item_id; // 商品对应的物品模板ID，由服务器设置
	int64_t player_money;  // 玩家身上有多少游戏币，由服务器设置
	int32_t player_cash;   // 玩家身上有多少人民币，由服务器设置
	int32_t player_score;  // 玩家身上有多少学分，由服务器设置
	int32_t money_cost;    // 本次购物需要花多少游戏币，由服务器设置
	int32_t cash_cost;     // 本次购物需要花多少人民币，由服务器设置
	int32_t score_cost;    // 本次购物需要花多少学分，由服务器设置

	NESTED_DEFINE(goods_id,
	              goods_count,
				  goods_remains,
				  goods_item_id,
				  player_money,
				  player_cash,
				  player_score,
				  money_cost,
				  cash_cost,
				  score_cost);
};

// SRV_DEF_STORAGE_TASK
struct storage_task
{
	int32_t storage_id; // 任务库的id
	TaskID  task_id;    // 所接的任务id
	NESTED_DEFINE(storage_id, task_id);
};

// SRV_DEF_ENHANCE
struct do_enhance
{
    int32_t enhance_gid;        // 需要附魔的附魔组ID
    int8_t cost_type;           // 消耗类型（0：学分，1：金币）
    int64_t player_own;         // 玩家当前身上的消耗代币数，由服务器设置
    int32_t protect_slot_num;   // 锁定的附魔位数目，由服务器设置
    int32_t enhance_cost;       // 本次附魔需要的消耗，由服务器设置
    int32_t count;              // 当前还剩余的附魔次数，由服务器设置
    NESTED_DEFINE(enhance_gid, cost_type, player_own, protect_slot_num, enhance_cost, count);
};

//#pragma pack()

} // namespace serviceDef

#endif // GAMED_CLIENT_PROTO_SERVICE_PARAM_H_
