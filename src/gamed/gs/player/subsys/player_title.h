#ifndef GAMED_GS_SUBSYS_PLAYER_TITLE_H_
#define GAMED_GS_SUBSYS_PLAYER_TITLE_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player称号子系统
 */
class PlayerTitle : public PlayerSubSystem
{
public:
	PlayerTitle(Player& player);
	virtual ~PlayerTitle();

	virtual void OnRelease();
	virtual void RegisterCmdHandler();
    
    bool SaveToDB(common::PlayerTitleData* pdata);
	bool LoadFromDB(const common::PlayerTitleData& data);

    void PlayerGetTitleData() const;
    void QueryTitleSkill(std::set<int32_t>& passive_skills) const;
    bool GainTitle(int32_t title_id);


protected:
    struct TitleListEntity
    {
        int32_t title_id;
    };

    void   CMDHandler_SwitchTitle(const C2G::SwitchTitle&);
    bool   SwitchTitle(int32_t old_title, int32_t new_title);


private:
    int32_t cur_title_;
    std::vector<TitleListEntity> title_list_;
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_TITLE_H_
