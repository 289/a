#include "player_bw_list.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/global_config.h"

#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

PlayerBWList::PlayerBWList(Player& player)
    : PlayerSubSystem(SUB_SYS_TYPE_BW_LIST, player)
{
    SAVE_LOAD_REGISTER(common::PlayerObjectBWList, PlayerBWList::SaveToDB, PlayerBWList::LoadFromDB);
}

PlayerBWList::~PlayerBWList()
{
}

bool PlayerBWList::LoadFromDB(const common::PlayerObjectBWList& data)
{
    if (data.black_list.size())
	{
		shared::net::ByteBuffer buf;
		buf.append(data.black_list.c_str(), data.black_list.size());
		buf >> obj_bw_list_.black_list;
	}

	if (data.white_list.size())
	{
		shared::net::ByteBuffer buf;
		buf.append(data.white_list.c_str(), data.white_list.size());
		buf >> obj_bw_list_.white_list;
	}

    return true;
}

bool PlayerBWList::SaveToDB(common::PlayerObjectBWList* pData)
{
    if (obj_bw_list_.black_list.size())
	{
		shared::net::ByteBuffer black_buf;
		black_buf << obj_bw_list_.black_list;
		pData->black_list.assign((char*)(black_buf.contents()), black_buf.size());
	}

	if (obj_bw_list_.white_list.size())
	{
		shared::net::ByteBuffer white_buf;
		white_buf << obj_bw_list_.white_list;
		pData->white_list.assign((char*)(white_buf.contents()), white_buf.size());
	}

    return true;
}

void PlayerBWList::OnEnterWorld()
{
    player_.sender()->ObjectBWList(obj_bw_list_);
}

void PlayerBWList::ModifyNPCBWList(int32_t templ_id, bool is_black, bool is_add)
{
	int n = 0;
	// black list
	if (is_black)
	{
		if (is_add) {
			if (obj_bw_list_.black_list.size() > kMaxNPCBWListSize)
			{
				LOG_ERROR << "玩家黑名单已满！role_id:" << player_.role_id();
				return;
			}
			n = obj_bw_list_.black_list.insert(templ_id).second;
		}
		else {
			n = obj_bw_list_.black_list.erase(templ_id);
		}
	}
	else // white list
	{
		if (is_add) {
			if (obj_bw_list_.white_list.size() > kMaxNPCBWListSize)
			{
				LOG_ERROR << "玩家白名单已满！role_id:" << player_.role_id();
				return;
			}
			n = obj_bw_list_.white_list.insert(templ_id).second;
		}
		else {
			n = obj_bw_list_.white_list.erase(templ_id);
		}
	}

	// is modify
	if (n != 0)
	{
		player_.sender()->ObjectBWListChange(templ_id, is_black, is_add);
	}
}

void PlayerBWList::ClearNPCBWList()
{
    obj_bw_list_.black_list.clear();
	obj_bw_list_.white_list.clear();
}

bool PlayerBWList::IsInBlackList(int32_t tid)
{
	BWListSet::iterator it = obj_bw_list_.black_list.find(tid);
	if (it != obj_bw_list_.black_list.end())
	{
		return true;
	}

	const dataTempl::GlobalConfigTempl* gconfig = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(gconfig);
	it = gconfig->obj_black_list.templ_id_set.find(tid);
	if (it !=  gconfig->obj_black_list.templ_id_set.end())
	{
		return true;
	}

	return false;
}

bool PlayerBWList::IsInWhiteList(int32_t tid)
{
	BWListSet::iterator it = obj_bw_list_.white_list.find(tid);
	if (it != obj_bw_list_.white_list.end())
	{
		return true;
	}

	return false;
}

} // namespace gamed
