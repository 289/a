#ifndef __GAMED_GS_GLOBAL_COOL_DOWN_H__
#define __GAMED_GS_GLOBAL_COOL_DOWN_H__

#include <map>
#include <vector>


namespace gamed {

class CoolDownMan
{
public:
	struct cd_entry
	{
		int32_t interval;	 // 冷却时间(ms)
		int64_t expire_time; // 到期时间(ms)
		cd_entry() {}
		cd_entry(int32_t n, int64_t time) : interval(n), expire_time(time) {}
	};

	typedef int32_t CoolDownID;
	typedef std::map<CoolDownID, cd_entry> CoolDownMap;

public:
	bool TestCoolDown(CoolDownID id);
	bool SetCoolDown(CoolDownID id, int interval);
	bool ClrCoolDown(CoolDownID id);
	void LoadFromData(const CoolDownMap& cd_list);
	void SaveToData(CoolDownMap& cd_list);

private:
	void ClearExpiredCoolDown();
	void Clear();

private:
	CoolDownMap cd_map_;
};

}; // namespace gamed

#endif // __GAMED_GS_GLOBAL_COOL_DOWN_H__
