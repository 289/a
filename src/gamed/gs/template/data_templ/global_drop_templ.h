#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_DROP_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_DROP_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class GlobalDropTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(GlobalDropTempl, TEMPL_TYPE_GLOBAL_DROP);
public:
	static const int kMaxMapCount     = 10;
	static const int kMaxMonsterCount = 100;

	inline void set_templ_id(TemplID id) { templ_id = id; }

	typedef int32_t MapID;
	struct DropLimit
	{
		BoundArray<MapID, kMaxMapCount>   map_ids_limit;         // 对哪些地图有效，为空表示对所有地图都有效。
		BoundArray<TemplID, kMaxMonsterCount> monster_ids_limit; // 对哪些怪物有效，为空表示对所有怪物都有效。
		int32_t monster_level_lower_limit;                       // 怪物等级下限。
		int32_t monster_level_upper_limit;                       // 怪物等级上限。
		int64_t monster_faction_limit;                           // 对怪物阵营的限制，为空表示对怪物阵营没有限制。
		NESTED_DEFINE(map_ids_limit, monster_ids_limit, monster_level_lower_limit, monster_level_upper_limit, monster_faction_limit);
	};

	/**
	 * day_type=DT_MON时，days表示在本月哪些天生效，有效范围为1-31。
	 * day_type=DT_WEEK时，days表示在本周哪些天生效，有效范围1-7。
	 */
	struct TimeDay
	{
		enum DayType
		{
			DT_INVALID,
			DT_MON,
			DT_WEEK,
			DT_MAX,
		};

		int8_t day_type;
		std::vector<int8_t> days;
		NESTED_DEFINE(day_type, days);
	};

	/**
	 * 此结构体用来确定开启和关闭的具体时间
	 */
	struct TimeEntry
	{
		int8_t min;  //几分
		int8_t hour; //几点
		NESTED_DEFINE(min, hour);
	};

	bool      is_active;            // 是否生效
	TimeDay   day_open;             // 生效日期
	TimeEntry time_open;            // 开启时间
	TimeEntry time_close;           // 关闭时间
	int32_t   drop_exp;             // 掉落经验值
	int32_t   drop_money;           // 掉落金钱数量
	TemplID   normal_droptable_id;  // 普通掉落包ID
	TemplID   special_droptable_id; // 特殊掉落包ID
	DropLimit drop_limit;           // 触发掉落的限制
	int32_t   drop_probability;     // 触发掉落的概率(万分数)

protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(is_active, day_open, time_open, time_close);
		MARSHAL_TEMPLVALUE(drop_exp, drop_money, normal_droptable_id, special_droptable_id);
		MARSHAL_TEMPLVALUE(drop_limit, drop_probability);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(is_active, day_open, time_open, time_close);
		UNMARSHAL_TEMPLVALUE(drop_exp, drop_money, normal_droptable_id, special_droptable_id);
		UNMARSHAL_TEMPLVALUE(drop_limit, drop_probability);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (day_open.day_type < TimeDay::DT_INVALID || day_open.day_type >= TimeDay::DT_MAX)
			return false;

		switch (day_open.day_type)
		{
			case TimeDay::DT_MON:
			{
				for (size_t i = 0; i < day_open.days.size(); ++ i)
					if (day_open.days[i] < 1 || day_open.days[i] > 31)
						return false;
			}
			break;
			case TimeDay::DT_WEEK:
			{
				for (size_t i = 0; i < day_open.days.size(); ++ i)
					if (day_open.days[i] < 1 || day_open.days[i] > 7)
						return false;
			}
			break;
			default:
			{
				//return false;
			}
			break;
		};

		if (drop_exp < 0 || drop_money < 0 || normal_droptable_id < 0 || special_droptable_id < 0)
			return false;

		if (drop_limit.monster_level_lower_limit < 0 || drop_limit.monster_level_lower_limit > 255)
			return false;

		if (drop_limit.monster_level_upper_limit < 0 || drop_limit.monster_level_upper_limit > 255)
			return false;

		if (drop_limit.monster_faction_limit < 0)
			return false;

		if (drop_probability < 0)
			return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_DROP_TEMPL_H_
