#ifndef GAMED_GS_SUBSYS_PLAYER_TALENT_H_
#define GAMED_GS_SUBSYS_PLAYER_TALENT_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace gamed
{

///
/// 天赋子系统
///
/// 天赋组和天赋的开启有三种方式：
/// 1) 默认就是开启状态；
/// 2) 通过任务来开启；
/// 3) 通过道具来开启；
///
/// 注意：天赋组的开启状态不影响天赋的开启
///

class PlayerSubSystem;
class PlayerTalent : public PlayerSubSystem
{
private:
	/*天赋信息*/
	struct TalentEntry
	{
		int32_t talent_id;
		int16_t level;
		int16_t tmp_level;
		int16_t max_level;

		TalentEntry():
			talent_id(0),
			level(0),
			tmp_level(0),
			max_level(0)
		{}
	};
	struct talent_finder
	{
		int32_t talent_id;
		talent_finder(int32_t id): talent_id(id) {}
		bool operator () (const TalentEntry& rhs) const
		{
			return talent_id == rhs.talent_id;
		}
	};

	/*天赋组信息*/
	struct TalentGroup
	{
		int32_t talent_group_id;
		mutable std::vector<TalentEntry> talent_list;
	};
	struct talent_group_finder
	{
		int32_t talent_group_id;
		talent_group_finder(int32_t id): talent_group_id(id) {}
		bool operator () (const TalentGroup& rhs) const
		{
			return talent_group_id == rhs.talent_group_id;
		}
	};

private:
	typedef std::vector<TalentEntry> TalentVec;
	typedef std::vector<TalentGroup> TalentGroupVec;
	TalentGroupVec talent_group_vec_; // 已激活天赋组列表

public:
	PlayerTalent(Player& player);
	~PlayerTalent();

	virtual void RegisterCmdHandler();

	bool LoadFromDB(const common::PlayerTalentData& data);
	bool SaveToDB(common::PlayerTalentData* pData);

	void CMDHandler_LevelUpTalent(const C2G::LevelUpTalent& cmd);

	bool IsTalentGroupExist(int32_t talent_group_id) const;
	bool IsTalentExist(int32_t talent_group_id, int32_t talent_id) const;
	void PlayerGetTalentData() const;
	void QueryTalentSkill(std::set<int32_t>& skills) const;
	bool OpenTalentGroup(int32_t talent_group_id);
	bool OpenTalent(int32_t talent_group_id, int32_t talent_id);
	bool CloseTalentGroup(int32_t talent_group_id);
	bool ManualLevelUp(int32_t talent_group_id, int32_t talent_id);
	bool SystemLevelUp(int32_t talent_group_id, int32_t talent_id);
	bool IncTempLevel(int32_t talent_group_id, int32_t talent_id);
	bool DecTempLevel(int32_t talent_group_id, int32_t talent_id);
    int32_t GetTalentLevel(int32_t talent_group_id, int32_t talent_id) const;

private:
	bool GetTalent(int32_t talent_group_id, int32_t talent_id, TalentEntry*& pTalent) const;
	bool GetTalentGroup(int32_t talent_group_id, TalentGroup*& pGroup);
	int32_t GetTalentSkill(int32_t talent_group_id, int32_t talent_id) const;
	void ActivateTalent(int32_t talent_group_id, int32_t talent_id) const;
	void DeActivateTalent(int32_t talent_group_id, int32_t talent_id) const;
	void NotifyClient(int32_t talent_group_id, int32_t talent_id, int level, int tmp_level) const;
};

};

#endif
