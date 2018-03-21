#include "player_achieve.h"

#include "gamed/game_module/achieve/include/achieve.h"
#include "gamed/client_proto/G2C_proto.h"

#include "gs/global/dbgprt.h"
#include "gs/player/achieve_if.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/global/timer.h"

namespace gamed {

using namespace common::protocol;

PlayerAchieve::PlayerAchieve(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_ACHIEVE, player)
{
	SAVE_LOAD_REGISTER(common::PlayerAchieveData, PlayerAchieve::SaveToDB, PlayerAchieve::LoadFromDB);
}

PlayerAchieve::~PlayerAchieve()
{
}

bool PlayerAchieve::LoadFromDB(const common::PlayerAchieveData& data)
{
	int32_t now = g_timer->GetSysTime();
	const std::string* pdata = &(data.achieve_data);
	if (!pdata->empty())
	{
		achieve_data_.AppendBuffer(pdata->c_str(), pdata->length());
		achieve_data_.Unmarshal();
	}

	pdata = &(data.finish_achieve);
	if (!pdata->empty())
	{
		finish_achieve_.AppendBuffer(pdata->c_str(), pdata->length());
		finish_achieve_.Unmarshal();
        finish_achieve_.LoadComplete();
	}

	PlayerAchieveIf achieve_if(&player_);
	achieve::PlayerLogin(&achieve_if, now);
    return true;
}

bool PlayerAchieve::SaveToDB(common::PlayerAchieveData* pData)
{
	achieve_data_.ClearContent();
	achieve_data_.Marshal();
	size_t size = achieve_data_.GetSize();
	const char* content = (const char*)(achieve_data_.GetContent());
	if (size > 0)
	{
		pData->achieve_data.assign(content, size);
	}

	finish_achieve_.ClearContent();
	finish_achieve_.Marshal();
	size = finish_achieve_.GetSize();
	content = (const char*)(finish_achieve_.GetContent());
	if (size > 0)
	{
		pData->finish_achieve.assign(content, size);
	}
	return true;
}

void PlayerAchieve::PlayerGetAchieveData()
{
	finish_achieve_.ClearContent();
	finish_achieve_.Marshal();
	achieve_data_.ClearContent();
	achieve_data_.Marshal();

	G2C::AchieveData packet;
	const char* content = (const char*)(finish_achieve_.GetContent());
	size_t size = finish_achieve_.GetSize();
	packet.finish_achieve.assign(content, size);

	content = (const char*)(achieve_data_.GetContent());
	size = achieve_data_.GetSize();
	packet.achieve_data.assign(content, size);

	player_.sender()->SendCmd(packet);
}

void PlayerAchieve::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::AchieveNotify, PlayerAchieve::CMDHandler_AchieveNotify);
}

void PlayerAchieve::RegisterMsgHandler()
{
}

void PlayerAchieve::CMDHandler_AchieveNotify(const C2G::AchieveNotify& packet)
{
	if (packet.databuf.size())
	{
		PlayerAchieveIf achieve_if(&player_);
		achieve::RecvClientNotify(&achieve_if, packet.type, packet.databuf.c_str(), packet.databuf.size());
	}
}

void PlayerAchieve::RefineAchieve(int32_t level)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::PlayerRefine(&achieve_if, level);
}

void PlayerAchieve::StorageTaskComplete(int32_t taskid, int8_t quality)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::StorageTaskComplete(&achieve_if, taskid, quality);
}

void PlayerAchieve::FinishInstance(int32_t ins_id)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::FinishInstance(&achieve_if, ins_id);
}

void PlayerAchieve::KillMonster(int32_t monster_id, int32_t monster_num)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::KillMonster(&achieve_if, monster_id, monster_num);
}

void PlayerAchieve::GainMoney(int32_t money)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::GainMoney(&achieve_if, money);
}

void PlayerAchieve::SpendMoney(int32_t money)
{
	PlayerAchieveIf achieve_if(&player_);
    achieve::SpendMoney(&achieve_if, money);
}

int32_t PlayerAchieve::GetInsFinishCount(int32_t ins_id) const
{
	PlayerAchieveIf achieve_if(&player_);
    return achieve::GetInsFinishCount(&achieve_if, ins_id);
}

} // namespace gamed
