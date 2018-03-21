#ifndef GAMED_GS_GLOBAL_MSG_PACK_DEF_H_
#define GAMED_GS_GLOBAL_MSG_PACK_DEF_H_

#include "game_def.h"


namespace gamed {

///
/// 注意这个文件里的结构，最好用message-inl.h里的MsgContentMarshal, MsgContentUnmarshal进行序列化反序列化
///

struct msgpack_get_attach_reply
{
	int64_t     mailid;
	int32_t     attach_cash;
	int32_t     attach_score;
	std::string attach_item;
	NESTED_DEFINE(mailid, attach_cash, attach_score, attach_item);
};

struct msgpack_announce_new_mail
{
	int64_t     mailid;
	int64_t     sender;
	int64_t     receiver;
	int32_t     attach_cash;
	int32_t     attach_score;
	std::string name;
	std::string title;
	std::string content;
	std::string attach_item;
	int32_t     time;
	NESTED_DEFINE(mailid, sender, receiver, attach_cash, attach_score, name, title, content, attach_item, time);
};

struct msgpack_teammeber_localins_reply
{
	int64_t member_roleid;
	bool    agreement;
	int32_t ins_group_id;
	std::vector<int32_t> ins_tid_vec;
	NESTED_DEFINE(member_roleid, agreement, ins_group_id, ins_tid_vec);
};

struct msgpack_ins_player_info
{
	int64_t role_id;
	std::string first_name;
	std::string mid_name;
	std::string last_name;
	int32_t level;
	int32_t cls;
	int32_t combat_value;
	NESTED_DEFINE(role_id, first_name, mid_name, last_name, level, cls, combat_value);
};

struct map_team_member_info
{
	map_team_member_info() { clear(); }

	void clear()
	{
		masterid     = 0;
		roleid       = 0;
		gender       = 0;
		cls          = 0;
		online       = false;
		level        = 0;
		combat_value = 0;
		first_name   = "";
		mid_name     = "";
		last_name    = "";
	}

	bool is_vacancy() const
	{
		return roleid <= 0;
	}

	int32_t masterid;
	int64_t roleid;
	int8_t  gender;
	int8_t  cls;
	bool    online;
	int32_t level;
	int32_t combat_value;
	std::string first_name;
	std::string mid_name;
	std::string last_name;
	NESTED_DEFINE(masterid, roleid, gender, cls, online, level, combat_value, first_name, mid_name, last_name);
};

struct msgpack_map_team_info
{
	msgpack_map_team_info() { clear(); }

	bool has_team() const
	{
		return team_id > 0;
	}

	void clear() 
	{ 
		team_id = 0;
		leader  = 0;
		for (size_t i = 0; i < kTeamMemberCount; ++i) {
			members[i].clear();
		}
	}

	int member_count() const
	{
		if (!has_team())
			return -1;

		int count = 0;
		for (size_t i = 0; i < members.size(); ++i) {
			if (members[i].roleid > 0) {
				++count;
			}
		}
		return count;
	}

	// 大于等于零表示成功
	int get_vacancy() const
	{
		int pos = -1;
		for (size_t i = 0; i < members.size(); ++i) 
		{
			if (members[i].roleid <= 0) 
			{
				pos = i;
				break;
			}
		}
		return pos;
	}

	// 大于零表示成功
	int get_gap_in_team(int advise_pos) const
	{
		if (members[advise_pos].roleid <= 0)
			return advise_pos;

		return get_vacancy();
	}

	bool find_member(int64_t roleid, int& pos) const
	{
		if (!has_team())
			return false;

		if (roleid <= 0)
			return false;

		for (size_t i = 0; i < members.size(); ++i)
		{
			if (members[i].roleid == roleid) {
				pos = i;
				return true;
			}
		}
		return false;
	}

    bool is_leader_online() const
    {
        int pos = 0;
        if (!find_member(leader, pos))
            return false;

        if (!members[pos].online)
            return false;

        return true;
    }

	bool is_leader(RoleID id) const
	{
		if (!has_team())
			return false;

		return id == leader;
	}

	bool get_first_online(RoleID& roleid) const
	{
		if (!has_team())
			return false;

		for (size_t i = 0; i < members.size(); ++i)
		{
			if (members[i].online && members[i].roleid > 0)
			{
				roleid = members[i].roleid;
				return true;
			}
		}
		return false;
	}

	int32_t team_id;
	RoleID  leader;
	shared::net::FixedArray<map_team_member_info, kTeamMemberCount> members;
	NESTED_DEFINE(team_id, leader, members);
};

struct msgpack_map_team_join
{
	int32_t pos;
	map_team_member_info info;
	NESTED_DEFINE(pos, info);
};

struct msgpack_map_team_tidy_pos
{
	shared::net::FixedArray<RoleID, kTeamMemberCount> members;
    NESTED_DEFINE(members);
};

struct msgpack_map_monster_killed
{
	struct monster_entry
	{
		int32_t monster_tid;
		int32_t monster_count;
		NESTED_DEFINE(monster_tid, monster_count);
	};

	std::vector<monster_entry> monster_list;
	NESTED_DEFINE(monster_list);
};

struct map_team_player_info
{
    int64_t role_id;
    int32_t masterid;
    int32_t link_id;
    int32_t sid_in_link;
    int8_t  gender;
    int8_t  cls;
    int32_t level;
    int32_t combat_value;
    std::string first_name;
    std::string mid_name;
    std::string last_name;
    NESTED_DEFINE(role_id, masterid, link_id, sid_in_link, gender, cls, level, combat_value, first_name, mid_name, last_name);
};

// 一方玩家发起申请或邀请
struct msgpack_map_team_join_team
{
    bool invite;   // true表示是邀请加入队伍，false表示是申请加入队伍
    map_team_player_info info;
    NESTED_DEFINE(invite, info);
};

struct msgpack_map_team_apply_for_join
{
    int64_t applicant;   // 申请入队的人
    int64_t respondent;  // 被申请人
    map_team_player_info info; // 申请人的信息
    map_team_player_info resp_info; // 被申请人的信息
    NESTED_DEFINE(applicant, respondent, info, resp_info);
};

struct duel_team_info
{
    RoleID  role_id;
    int32_t link_id;
    int32_t sid_in_link;
    int32_t combat_value;
    NESTED_DEFINE(role_id, link_id, sid_in_link, combat_value);
};

struct msgpack_duel_prepare
{
    std::vector<duel_team_info> members;
    NESTED_DEFINE(members);
};

// world_boss collect player info
struct msgpack_wb_player_info
{
    msgpack_wb_player_info() {
        clear();
    }

    void clear()
    {
        roleid       = 0;
        masterid     = 0;
        cls          = 0;
        gender       = 0;
        level        = 0;
        combat_value = 0;
        first_name.clear();
        middle_name.clear();
        last_name.clear();
    }

    RoleID     roleid;
    int32_t    masterid;
    int16_t    cls;
    int16_t    gender;
    int32_t    level;
    int32_t    combat_value;
    std::string first_name;
    std::string middle_name;
    std::string last_name;
    NESTED_DEFINE(roleid, masterid, cls, gender, level, combat_value, first_name, middle_name, last_name);
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_MSG_PACK_DEF_H_
