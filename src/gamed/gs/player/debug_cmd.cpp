#include "player_ctrl.h"

#include "game_module/task/include/task.h"
#include "game_module/task/include/task_data.h"
#include "game_module/achieve/include/achieve_data.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_templ.h"
#include "gs/template/data_templ/battleground_templ.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/game_util.h"
#include "gs/global/randomgen.h"
#include "gs/item/item_data.h"
#include "gs/item/item.h"

// proto
#include "common/protocol/gen/G2M/instance_msg.pb.h"

#include "player.h"
#include "player_sender.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

#define DEBUG_CMD_HANDLER(packet, handler) \
	ASSERT(packet::TypeNumber() >= C2G_DEBUG_CMD_LOWER_LIMIT && packet::TypeNumber() <= C2G_DEBUG_CMD_UPPER_LIMIT); \
	debug_cmd_disp_.Register<packet>(BIND_MEM_CB(&handler, this));

namespace {

    static const int kMaxRandomCount = 5000000;

} // Anonymous


///
/// register debug cmd handlers
/// 
void PlayerController::StartupDebugCmdDispRegister()
{
	DEBUG_CMD_HANDLER(C2G::DebugCommonCmd, PlayerController::DebugCommonCmd);
	DEBUG_CMD_HANDLER(C2G::DebugChangePlayerPos, PlayerController::DebugChangePlayerPos);
    DEBUG_CMD_HANDLER(C2G::DebugChangeFirstName, PlayerController::DebugChangeFirstName);
}


/**
 * @brief DebugCommonCmd 
 *    （1）CommonDebugCmd是通用的debug命令协议，可以带8个参数（int64_t或double）
 */
void PlayerController::DebugCommonCmd(const C2G::DebugCommonCmd& cmd)
{
#define CHECK_AND_POP_PARAM(cmd, p) \
	if (!C2G::DebugCommonCmd::PopParam(cmd, p)) \
	{ \
		__PRINTF("CommonDebug命令:%d 的参数类型不正确！", cmd.type); \
		return; \
	}

	// process cmd
	switch (cmd.type)
	{
		case C2G::CDC_CLR_SERVICE_COOLDOWN:
			{
				struct dccp
				{
					int64_t service_type;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.service_type);
			}
			break;

		case C2G::CDC_OPEN_TALENT_GROUP:
			{
				struct dccp
				{
					int64_t talent_group_id;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.talent_group_id);
				player_.OpenTalentGroup(param.talent_group_id);
			}
			break;

		case C2G::CDC_CLOSE_TALENT_GROUP:
			{
				struct dccp
				{
					int64_t talent_group_id;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.talent_group_id);
				player_.CloseTalentGroup(param.talent_group_id);
			}
			break;

        case C2G::CDC_OPEN_ENHANCE_SLOT:
            {
				struct dccp
				{
					int64_t mode;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.mode);
				player_.OpenEnhanceSlot(param.mode);
            }
            break;

        case C2G::CDC_PROTECT_ENHANCE_SLOT:
            {
				struct dccp
				{
					int64_t protect;
                    int64_t slot_index;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.protect);
				CHECK_AND_POP_PARAM(cmd, param.slot_index);
                if (param.protect == 0)
                {
				    player_.UnProtectEnhanceSlot(param.slot_index);
                }
                else
                {
                    player_.ProtectEnhanceSlot(param.slot_index);
                }
            }
            break;

        case C2G::CDC_DO_ENHANCE:
            {
				struct dccp
				{
                    int64_t enhance_gid;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.enhance_gid);
				player_.DoEnhance(param.enhance_gid, 0);
            }
            break;

		case C2G::CDC_GAIN_MONEY:
			{
				struct dccp
				{
					int64_t money;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.money);
				player_.GainMoney(param.money);
			}
			break;

		case C2G::CDC_GAIN_EXP:
			{
				struct dccp
				{
					int64_t exp;

				} param;

				CHECK_AND_POP_PARAM(cmd, param.exp);
				player_.IncExp(param.exp);
			}
			break;

		case C2G::CDC_GAIN_ITEM:
			{
				struct dccp
				{
					int64_t item_id;
					int64_t item_count;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.item_id);
				CHECK_AND_POP_PARAM(cmd, param.item_count);
				player_.GainItem(param.item_id, param.item_count);
			}
			break;

		case C2G::CDC_CHANGE_PLAYER_POS:
			{
				struct dccp
				{
					int64_t world_id;
					double  x;
					double  y;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.world_id);
				CHECK_AND_POP_PARAM(cmd, param.x);
				CHECK_AND_POP_PARAM(cmd, param.y);

				if (!IS_NORMAL_MAP(param.world_id))
					return;

				msg_player_region_transport msg_param;
				msg_param.source_world_id = player_.world_id();
				msg_param.target_world_id = param.world_id;
				msg_param.target_pos.x    = param.x;
				msg_param.target_pos.y    = param.y;
				player_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, player_.object_xid(), &param, sizeof(param));
			}
			break;

		case C2G::CDC_DEL_ITEM:
			{
				struct dccp
				{
					int64_t item_id;
					int64_t item_count;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.item_id);
				CHECK_AND_POP_PARAM(cmd, param.item_count);
				player_.TakeOutItem(param.item_id, param.item_count);
			}
			break;

		case C2G::CDC_TASK_CMD:
			{
				struct dccp
				{
					int64_t task_id;
					int64_t type;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.task_id);
				CHECK_AND_POP_PARAM(cmd, param.type);
				
				int32_t task_id = param.task_id;
				if (param.type == 0) // 删除任务
				{
					if (task_id == 0)
					{
						player_.GetActiveTask()->task_list.clear();
						player_.GetActiveTask()->storage_num = 0;
						player_.GetFinishTask()->task_list.clear();
						player_.GetFinishTimeTask()->task_list.clear();
						player_.ClearNPCBWList();
					}
					else
					{
						player_.GetActiveTask()->Erase(task_id);
						player_.GetFinishTask()->Erase(task_id);
						player_.GetFinishTimeTask()->Erase(task_id);
					}
				}
				else if (param.type == 1) // 发放任务
				{
					player_.DeliverTask(task_id);
				}
				else if (param.type == 2) // 完成任务
				{
					player_.GetActiveTask()->Erase(task_id);
					task::TaskEntry entry;
					entry.id = task_id;
					entry.SetFinish();
					entry.deliver_time = 0;
					entry.finish_time = 0;				
					player_.GetFinishTask()->AddEntry(entry);
				}
			}
			break;

		case C2G::CDC_TRANSFER_CLS:
			{
				struct dccp
				{
					int64_t dest_cls;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.dest_cls);
				player_.TransferCls(param.dest_cls);
			}
			break;

		case C2G::CDC_MODIFY_BW_LIST:
			{
				struct dccp
				{
					int64_t is_black;
					int64_t is_add;
					int64_t templ_id;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.is_black);
				CHECK_AND_POP_PARAM(cmd, param.is_add);
				CHECK_AND_POP_PARAM(cmd, param.templ_id);
				player_.ModifyNPCBWList(param.templ_id, param.is_black, param.is_add);
			}
			break;

		case C2G::CDC_GAIN_CAT_VISION_POWER:
			{
				struct dccp
				{
					int64_t power;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.power);
				player_.GainCatVisionPower(param.power);
			}
			break;

		case C2G::CDC_ADD_CASH:
			{
				struct dccp
				{
					int64_t cash_add;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.cash_add);
				player_.AddCashByGame(param.cash_add);
			}
			break;

		case C2G::CDC_CHANGE_GENDER:
			{
				struct dccp
				{
					int64_t gender;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.gender);
				player_.TransferGender(param.gender);
			}
			break;

		case C2G::CDC_MAP_ELEM_ON_OFF:
			{
				struct dccp
				{
					int64_t elem_id;
					int64_t on_off;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.elem_id);
				CHECK_AND_POP_PARAM(cmd, param.on_off);
				bool open = (param.on_off == 0) ? false : true;
				player_.CtrlMapElement(param.elem_id, open);
			}
			break;

		case C2G::CDC_SEND_SYS_MAIL:
			{
				/*struct dccp
				{
					int64_t score;
					int64_t item_id;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.score);
				CHECK_AND_POP_PARAM(cmd, param.item_id);

				playerdef::SysMail sysmail;
				sysmail.attach_score = param.score;
				sysmail.sender = "GM";
				sysmail.title = "test sys mail";
				sysmail.content = "hello world";
				playerdef::MailAttach attach;
				attach.id = param.item_id;
				attach.count = 1;
				sysmail.attach_list.push_back(attach);
				shared::net::ByteBuffer buf;
				sysmail.Pack(buf);
				player_.SendSysMail(buf);*/
			}
			break;

		case C2G::CDC_PLAYER_QUIT_INS:
			{
				if (IS_INS_MAP(player_.world_id()))
				{
					player_.SendMsg(GS_MSG_PLAYER_QUIT_INS, player_.object_xid());
				}
			}
			break;

		case C2G::CDC_UPDATE_TASK_STORAGE:
			{
				struct dccp
				{
					int64_t storage_id;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.storage_id);
				player_.UpdateTaskStorage(param.storage_id);
			}
			break;

		case C2G::CDC_GAIN_SCORE:
			{
				struct dccp
				{
					int64_t score;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.score);
				player_.GainScore(param.score);
			}		
			break;

		case C2G::CDC_ENTER_INSTANCE:
			{
				struct dccp
				{
					int64_t ins_tid;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.ins_tid);
				const dataTempl::InstanceTempl* pins_templ = s_pDataTempl->QueryDataTempl<InstanceTempl>(param.ins_tid);
				if (pins_templ == NULL)
				{
					__PRINTF("Debug命令传副本：没有找到对应的副本模板，id=%ld", param.ins_tid);
					return;
				}
                
                msg_ins_transfer_prepare ins_param;
                ins_param.ins_templ_id = param.ins_tid;
                ins_param.request_type = G2M::IRT_UI_SOLO;
                player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
            }
            break;

        case C2G::CDC_GAIN_TITLE:
            {
                struct dccp
                {
                    int64_t title_id;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.title_id);
                player_.GainTitle(param.title_id);
            }
            break;

        case C2G::CDC_OPEN_REPUTATION:
            {
                struct dccp
                {
                    int64_t rep_id;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.rep_id);
                player_.OpenReputation(param.rep_id);
            }
            break;

        case C2G::CDC_MODIFY_REPUTATION:
            {
                struct dccp
                {
                    int64_t rep_id;
                    int64_t delta;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.rep_id);
				CHECK_AND_POP_PARAM(cmd, param.delta);
                player_.ModifyReputation(param.rep_id, param.delta);
            }
            break;

        case C2G::CDC_MODIFY_UI:
            {
                struct dccp
                {
                    int64_t ui_id;
                    int64_t show;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.ui_id);
				CHECK_AND_POP_PARAM(cmd, param.show);
                player_.ModifyUIConf(param.ui_id, param.show);
            }
            break;

        case C2G::CDC_OPEN_STAR:
            {
                struct dccp
                {
                    int64_t star_id;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.star_id);
                player_.OpenStar(param.star_id);
            }
            break;

        case C2G::CDC_ACTIVATE_SPARK:
            {
                struct dccp
                {
                    int64_t star_id;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.star_id);
                player_.ActivateSpark(param.star_id);
            }
            break;

        case C2G::CDC_ENTER_BATTLEGROUND:
            {
                struct dccp
                {
                    int64_t bg_tid;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.bg_tid);
                player_.EnterPveBattleGround(param.bg_tid);
            }
            break;

        case C2G::CDC_CLOSE_CUR_BG:
            {
                if (IS_BG_MAP(player_.world_id()))
                {
                    plane_msg_sys_close_bg param;
                    param.sys_type  = plane_msg_sys_close_bg::ST_SCRIPT;
                    param.bg_result = plane_msg_sys_close_bg::FACTION_A_VICTORY;
                    player_.SendPlaneMsg(GS_PLANE_MSG_SYS_CLOSE_BG, &param, sizeof(param));
                }
            }
            break;

        case C2G::CDC_PRINT_ACTIVE_TASK:
            {
                std::string chat_msg;
                chat_msg = "Active_Task_List: ";
                const task::ActiveTask* ptask = player_.GetActiveTask();
                task::EntryMap::const_iterator it = ptask->task_list.begin();
                for (; it != ptask->task_list.end(); ++it)
                {
                    chat_msg += itos(it->first) + ", ";
                }
                player_.Say(chat_msg);
            }
            break;

        case C2G::CDC_PRINT_HIDE_ITEM:
            {
                std::string chat_msg;
                chat_msg = "Hide_Item_List size = ";
                std::vector<itemdata> list;
                if (player_.GetItemListDetail(Item::HIDE_INV, list))
                {
                    chat_msg += itos(list.size()) + ":  ";
                    for (size_t i = 0; i < list.size(); ++i)
                    {
                        chat_msg += "[" + itos(list[i].index) + ", ";
                        chat_msg += itos(list[i].id) + ":" + itos(list[i].count) + "]";
                        chat_msg += "  ";
                    }
                }
                else
                {
                    chat_msg += itos(list.size()) + ": error!";
                }
                player_.Say(chat_msg);
            }
            break;

        case C2G::CDC_ACHIEVE_CMD:
            {
				struct dccp
				{
					int64_t achieve_id;
					int64_t type;
				} param;

				CHECK_AND_POP_PARAM(cmd, param.achieve_id);
				CHECK_AND_POP_PARAM(cmd, param.type);
				
				if (param.type == 0) // 删除成就
				{
					if (param.achieve_id == 0)
					{
                        achieve::ActiveAchieve* active = player_.GetActiveAchieve();
                        achieve::FinishAchieve* finish = player_.GetFinishAchieve();
                        achieve::EntryMap::iterator it = finish->achieve_list.begin();
                        for (; it != finish->achieve_list.end();)
                        {
                            active->achieve_list[it->first] = it->second.templ;
                            finish->achieve_list.erase(it++);
                        }
                        achieve::AchieveData* data = player_.GetAchieveData();
                        data->data.clear();
					}
				}
			}
            break;

        case C2G::CDC_SPOT_MAPELEM_TELEPORT:
            {
                struct dccp
                {
                    int64_t elem_id;
                    double  pos_x;
                    double  pos_y;
                    int64_t dir;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.elem_id);
                CHECK_AND_POP_PARAM(cmd, param.pos_x);
                CHECK_AND_POP_PARAM(cmd, param.pos_y);
                CHECK_AND_POP_PARAM(cmd, param.dir);

                if (IS_INS_MAP(player_.world_id()) || IS_BG_MAP(player_.world_id()))
                {
                    XID target = player_.world_xid();
                    plane_msg_spot_mapelem_teleport send_param;
                    send_param.elem_id = param.elem_id;
                    send_param.pos_x   = param.pos_x;
                    send_param.pos_y   = param.pos_y;
                    send_param.dir     = param.dir;

                    MSG msg;
                    BuildMessage(msg, GS_PLANE_MSG_SPOT_MAPELEM_TELEPORT, target, target, 
                                 0, &send_param, sizeof(send_param), player_.pos());
                    Gmatrix::SendWorldMsg(msg);
                }
            }
            break;

        case C2G::CDC_SPOT_MONSTER_MOVE:
            {
                struct dccp
                {
                    int64_t elem_id;
                    double pos_x;
                    double pos_y;
                    double speed;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.elem_id);
                CHECK_AND_POP_PARAM(cmd, param.pos_x);
                CHECK_AND_POP_PARAM(cmd, param.pos_y);
                CHECK_AND_POP_PARAM(cmd, param.speed);

                if (IS_INS_MAP(player_.world_id()) || IS_BG_MAP(player_.world_id()))
                {
                    XID target = player_.world_xid();
                    plane_msg_spot_monster_move send_param;
                    send_param.elem_id = param.elem_id;
                    send_param.pos_x   = param.pos_x;
                    send_param.pos_y   = param.pos_y;
                    send_param.speed   = param.speed;

                    MSG msg;
                    BuildMessage(msg, GS_PLANE_MSG_SPOT_MONSTER_MOVE, target, target, 
                                 0, &send_param, sizeof(send_param), player_.pos());
                    Gmatrix::SendWorldMsg(msg);
                }
            }
            break;

        case C2G::CDC_RESET_PRIMARY_PROP_RF:
            {
                player_.ResetPrimaryPropRF();
            }
            break;

        case C2G::CDC_NPC_FRIEND:
            {
                struct dccp
                {
                    int64_t npc_tid;
                    int64_t op;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.npc_tid);
                CHECK_AND_POP_PARAM(cmd, param.op);

                switch (param.op)
                {
                case 0:
                    player_.AddNPCFriend(param.npc_tid);
                    break;
                case 1:
                    player_.DelNPCFriend(param.npc_tid);
                    break;
                case 2:
                    player_.OnlineNPCFriend(param.npc_tid);
                    break;
                case 3:
                    player_.OfflineNPCFriend(param.npc_tid);
                    break;
                default:
                    break;
                }
            }
            break;

        case C2G::CDC_PET_EXP:
            {
                struct dccp
                {
                    int64_t pet_exp;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.pet_exp);
                player_.GainPetExp(param.pet_exp);
            }
            break;

        case C2G::CDC_CLOSE_CUR_INS:
            {
                if (IS_INS_MAP(player_.world_id()))
                {
                    plane_msg_sys_close_ins param;
                    param.sys_type   = plane_msg_sys_close_ins::ST_SCRIPT;
                    param.ins_result = plane_msg_sys_close_ins::PLAYER_FAILURE;
                    player_.SendPlaneMsg(GS_PLANE_MSG_SYS_CLOSE_INS, &param, sizeof(param));
                }
            }
            break;

        case C2G::CDC_RESET_PUNCH_CARD:
            {
                player_.ResetPunchCard();
            }
            break;

        case C2G::CDC_OPEN_MOUNT_EQUIP:
            {
                struct dccp
                {
                    int64_t equip_index;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.equip_index);
                player_.OpenMountEquip(param.equip_index);
            }
            break;

        case C2G::CDC_SET_GEVENT_NUM:
            {
                struct dccp
                {
                    int64_t gevent_gid;
                    int64_t gevent_num;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.gevent_gid);
                CHECK_AND_POP_PARAM(cmd, param.gevent_num);
                player_.SetGeventNum(param.gevent_gid, param.gevent_num);
            }
            break;

        case C2G::CDC_SET_PARTICIPATION:
            {
                struct dccp
                {
                    int64_t participation;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.participation);
                player_.SetParticipation(param.participation);
            }
            break;

        case C2G::CDC_RESET_PARTI_AWARD:
            {
                player_.ClearParticipationAward();
            }
            break;

        case C2G::CDC_CLOSE_STAR:
            {
                struct dccp
                {
                    int64_t star_id;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.star_id);
                player_.CloseStar(param.star_id);
            }
            break;

        case C2G::CDC_PRINT_FINISH_TASK:
            {
                std::string chat_msg;
                chat_msg = "Finish_Task_List: ";
                const task::FinishTask* ptask = player_.GetFinishTask();
                task::EntryMap::const_iterator it = ptask->task_list.begin();
                for (; it != ptask->task_list.end(); ++it)
                {
                    chat_msg += itos(it->first) + ", ";
                }
                player_.Say(chat_msg);
            }
            break;

        case C2G::CDC_ADD_PPRF_CUR_ENERGY:
            {
                player_.AddPrimaryPropRFCurEnergy();
            }
            break;

        case C2G::CDC_CLEAR_BOSS_CHALLENGE:
            {
                struct dccp
                {
                    int64_t challenge_id;
                } param;
                CHECK_AND_POP_PARAM(cmd, param.challenge_id);
                player_.ClearBossChallenge(param.challenge_id);
            }
            break;

        case C2G::CDC_PLAYER_SUICIDE:
            {
                player_.DecHP(player_.GetHP());
            }
            break;

        case C2G::CDC_RANDOM_TEST_A:
            {
                struct dccp
                {
                    double  prob;
                    int64_t count;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.prob);
                CHECK_AND_POP_PARAM(cmd, param.count);

                if (param.prob < 0 || param.prob > 1)
                    return;

                if (param.count < 0 || param.count > kMaxRandomCount)
                    return;

                int rand_succ = 0;
                for (int i = 0; i < param.count; ++i)
                {
                    if (mrand::RandSelectF(param.prob))
                        rand_succ++;
                }
                
                std::string chat_msg;
                chat_msg  = "Random times: " + itos(param.count) + "\n";
                chat_msg += "Probability less-than " + ftos(param.prob) + ": " + itos(rand_succ) + "\n";
                chat_msg += "Probability greater-than " + ftos(param.prob) + ": " + itos(param.count - rand_succ);
                player_.Say(chat_msg);
            }
            break;

        case C2G::CDC_RANDOM_TEST_B:
            {
                struct dccp
                {
                    int64_t prob;
                    int64_t count;
                } param;

                CHECK_AND_POP_PARAM(cmd, param.prob);
                CHECK_AND_POP_PARAM(cmd, param.count);

                if (param.prob < 0 || param.prob > 10000)
                    return;

                if (param.count < 0 || param.count > kMaxRandomCount)
                    return;

                int rand_succ = 0;
                for (int i = 0; i < param.count; ++i)
                {
                    if (mrand::RandSelect(param.prob))
                        rand_succ++;
                }
                
                std::string chat_msg;
                chat_msg  = "Random times: " + itos(param.count) + "\n";
                chat_msg += "Probability less-than " + itos(param.prob) + ": " + itos(rand_succ) + "\n";
                chat_msg += "Probability greater-than " + itos(param.prob) + ": " + itos(param.count - rand_succ);
                player_.Say(chat_msg);
            }
            break;

        default:
			__PRINTF("DebugCommonCmd() CommonDebug:%d 没有处理的代码", cmd.type);
			break;
	}

#undef CHECK_AND_POP_PARAM
}


///
/// -------- debug cmd --------
///
void PlayerController::DebugChangePlayerPos(const C2G::DebugChangePlayerPos& cmd)
{
	if (!IS_NORMAL_MAP(cmd.world_id) &&
		player_.world_id() != cmd.world_id)
	{
		return;
	}

	msg_player_region_transport param;
	param.source_world_id = player_.world_id();
	param.target_world_id = cmd.world_id;
	param.target_pos.x    = cmd.x;
	param.target_pos.y    = cmd.y;
	player_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, player_.object_xid(), &param, sizeof(param));
}

void PlayerController::DebugChangeFirstName(const C2G::DebugChangeFirstName& cmd)
{
    player_.ChangeName(playerdef::NT_FIRST_NAME, cmd.first_name);
}

} // namespace gamed
