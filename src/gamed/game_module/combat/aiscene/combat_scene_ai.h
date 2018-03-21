#ifndef __GAME_MODULE_COMBAT_SCENE_AI_H__
#define __GAME_MODULE_COMBAT_SCENE_AI_H__


namespace combat
{

/**
 * @class CombatSceneAI
 * @brief 战斗场景策略
 */
struct CLuaState;
class CombatSceneAI
{
private:
	Combat* combat_;
	CLuaState* lstate_;

public:
	explicit CombatSceneAI(Combat* combat);
	~CombatSceneAI();

	bool Init();
	bool Resume();
	void Release();

	bool IsNormalStatus() const;
    bool IsYieldStatus() const;

	void OnCombatStart();
	void OnCombatEnd();
	void OnRoundStart(int32_t unit_id);
	void OnRoundEnd(int32_t unit_id);
	void OnDamage(int32_t mob_uid, int32_t mob_tid);
};

};

#endif // __GAME_MODULE_COMBAT_SCENE_AI_H__
