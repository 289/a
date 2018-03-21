#ifndef ACHIEVE_SERVER_ACHIEVE_H_
#define ACHIEVE_SERVER_ACHIEVE_H_

#include "achieve_types.h"

namespace achieve
{

// 初始化
bool InitAchieveSys(const char* file);

// 接收客户端发送的成就消息
void RecvClientNotify(Player* player, uint16_t type, const char* buff, size_t size);

// 成就系统数据变化
void PlayerLogin(Player* player, int32_t now);
void PlayerRefine(Player* player, int32_t level);
void StorageTaskComplete(Player* player, int32_t taskid, int8_t quality);
void KillMonster(Player* player, int32_t mob_id, int32_t mob_count);
void FinishInstance(Player* player, int32_t ins_id);
void GainMoney(Player* player, int32_t money);
void SpendMoney(Player* player, int32_t money);

// 获取成就数据
int32_t GetInsFinishCount(Player* player, int32_t ins_id);

} // namespace achieve

#endif // ACHIEVE_SERVER_ACHIEVE_H_
