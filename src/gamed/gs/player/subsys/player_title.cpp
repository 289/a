#include "player_title.h"

#include "common/obj_data/gen/player/title_data.pb.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/title_templ.h"
#include "gs/global/glogger.h"
#include "gs/global/dbgprt.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

using namespace dataTempl;

PlayerTitle::PlayerTitle(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_TITLE, player),
      cur_title_(0)
{
	SAVE_LOAD_REGISTER(common::PlayerTitleData, PlayerTitle::SaveToDB, PlayerTitle::LoadFromDB);
}

PlayerTitle::~PlayerTitle()
{
}

void PlayerTitle::OnRelease()
{
}

bool PlayerTitle::SaveToDB(common::PlayerTitleData* pdata)
{
    pdata->cur_title = cur_title_;
    if (title_list_.size() > 0)
    {
        common::scalable::TitleData scalable_data;
        for (size_t i = 0; i < title_list_.size(); ++i)
        {
            common::scalable::TitleData::Entity* ent = scalable_data.add_title_list();
            ent->set_title_id(title_list_[i].title_id);
        }

        std::string& tmpbuf = pdata->title_data;
        tmpbuf.resize(scalable_data.ByteSize());
        if (!scalable_data.SerializeToArray((void*)tmpbuf.c_str(), tmpbuf.size()))
        {
            GLog::log("PlayerTitle::SaveToDB Serialize error! roleid:%ld", player_.role_id());
            return false;
        }
    }
    return true;
}

bool PlayerTitle::LoadFromDB(const common::PlayerTitleData& data)
{
    cur_title_ = data.cur_title;

    //获取称号列表
    if (data.title_data.size() > 0)
    {
        common::scalable::TitleData scalable_data;
        const std::string& bufRef = data.title_data;
        if (!scalable_data.ParseFromArray(bufRef.c_str(), bufRef.size()))
        {
            GLog::log("PlayerTitle::LoadFromDB error! roleid: ", player_.role_id());
            return false;
        }
        for (int i = 0; i < scalable_data.title_list_size(); ++i)
        {
            TitleListEntity ent;
            ent.title_id = scalable_data.title_list(i).title_id();
            title_list_.push_back(ent);
        }
    }
    
    //让全部称号生效
    for (size_t i = 0; i < title_list_.size(); ++ i)
    {
        int32_t title_templ_id = title_list_[i].title_id;
        const dataTempl::TitleTempl* tpl = s_pDataTempl->QueryDataTempl<const dataTempl::TitleTempl>(title_templ_id);
        if (!tpl)
        {
            LOG_ERROR << "玩家 " << player_.role_id() << " 称号模板ID非法!!! title_templ_id: " << title_templ_id;
            continue;
        }

        for (size_t i = 0; i < tpl->addon_props.size(); ++ i)
        {
            player_.IncPropPoint(i, tpl->addon_props[i]);
        }
    }

    if (cur_title_ > 0)
    {
        //设置称号标记
        player_.visible_state().SetTitleFlag(cur_title_);
    }
    return true;
}

void PlayerTitle::PlayerGetTitleData() const
{
    G2C::TitleData packet;
    packet.cur_title = cur_title_;
    for (size_t i = 0; i < title_list_.size(); ++i)
    {
        packet.title_list.push_back(title_list_[i].title_id);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerTitle::QueryTitleSkill(std::set<int32_t>& passive_skills) const
{
    const TitleTempl* tpl = s_pDataTempl->QueryDataTempl<const TitleTempl>(cur_title_);
    if (!tpl)
        return;

    for (size_t i = 0; i < tpl->addon_passive_skills.size(); ++i)
    {
        passive_skills.insert(tpl->addon_passive_skills[i]);
    }
}

bool PlayerTitle::GainTitle(int32_t title_id)
{
    const dataTempl::TitleTempl* tpl = s_pDataTempl->QueryDataTempl<const dataTempl::TitleTempl>(title_id);
    if (!tpl)
        return false;
    
    for (size_t i = 0; i < title_list_.size(); ++ i)
    {
        if (title_list_[i].title_id == title_id)
        {
		    LOG_ERROR << "玩家 " << player_.role_id() << " 获得重复的称号!!! title_id: " << title_id;
            return false;
        }
    }

    //保存新获得的称号
    TitleListEntity ent;
    ent.title_id = title_id;
    title_list_.push_back(ent);

    //重置当前称号
    cur_title_ = title_id;

    //新获得的称号生效
    for (size_t i = 0; i < tpl->addon_props.size(); ++ i)
    {
        player_.IncPropPoint(i, tpl->addon_props[i]);
    }

    player_.visible_state().SetTitleFlag(cur_title_);

    // 发给自己
    G2C::GainTitle packet;
    packet.title_id = title_id;
    player_.sender()->SendCmd(packet);

    // 广播给别人
    player_.sender()->PlayerUpdateTitle(title_id);
    GLog::log("玩家 %ld 获得新称号，title_id:%ld", player_.role_id(), title_id);
    return true;
}

bool PlayerTitle::SwitchTitle(int32_t old_title, int32_t new_title)
{
    if (old_title < 0 || new_title < 0 || old_title == new_title)
        return false;

    if (cur_title_ != old_title)
        return false;

    const dataTempl::TitleTempl* pOldTpl = NULL;
    const dataTempl::TitleTempl* pNewTpl = NULL;
    if (old_title > 0)
    {
        pOldTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TitleTempl>(old_title);
        if (!pOldTpl)
            return false;
    }

    if (new_title > 0)
    {
        pNewTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TitleTempl>(new_title);
        if (!pNewTpl)
            return false;
    }
    
    bool found_title = false;
    for (size_t i = 0; i < title_list_.size(); ++i)
    {
        if (title_list_[i].title_id == new_title)
        {
            found_title = true;
            break;
        }
    }

    // success
    if (!found_title)
    {
        LOG_ERROR << "玩家 " << player_.role_id() << " 切换称号失败!!! old_title: " << old_title 
            << ", new_title: " << new_title;
        return false;
    }
    cur_title_ = new_title;

    // 设置标记
    player_.visible_state().SetTitleFlag(cur_title_);

    // 发给自己
    G2C::SwitchTitle_Re packet;
    packet.old_title = old_title;
    packet.new_title = new_title;
    player_.sender()->SendCmd(packet);

    // 广播给别人
    player_.sender()->PlayerUpdateTitle(new_title);
    return true;
}

void PlayerTitle::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::SwitchTitle, PlayerTitle::CMDHandler_SwitchTitle);
}

void PlayerTitle::CMDHandler_SwitchTitle(const C2G::SwitchTitle& cmd)
{
    if (!SwitchTitle(cmd.old_title, cmd.new_title))
    {
        __PRINTF("玩家：%ld切换称号失败！", player_.role_id());
    }
}

} // namespace gamed
