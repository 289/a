#ifndef GAMED_GS_SUBSYS_PLAYER_BW_LIST_H_
#define GAMED_GS_SUBSYS_PLAYER_BW_LIST_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief 玩家黑白名单 - black white list
 */
class PlayerBWList : public PlayerSubSystem
{
public:
	PlayerBWList(Player& player);
	~PlayerBWList();

	virtual void OnEnterWorld();

    bool LoadFromDB(const common::PlayerObjectBWList& data);
	bool SaveToDB(common::PlayerObjectBWList* pData);

    void ModifyNPCBWList(int32_t templ_id, bool is_black, bool is_add);
    bool IsInBlackList(int32_t tid);
    bool IsInWhiteList(int32_t tid);
	void ClearNPCBWList();


private:
    typedef std::set<int32_t> BWListSet;
	playerdef::ObjectBWList  obj_bw_list_; // 对象的黑白名单
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_BW_LIST_H_
