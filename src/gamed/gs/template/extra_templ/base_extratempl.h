#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_BASE_TEMPL_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_BASE_TEMPL_H_

#include "shared/base/singleton.h"

#include "templ_util.h"
#include "templ_types.h"


namespace extraTempl {

using namespace shared::net;

enum DATA_TEMPLATE_TYPE
{
	TEMPL_TYPE_INVALID = 0,
	TEMPL_TYPE_MONSTER_GROUP_TEMPL,
	TEMPL_TYPE_BATTLE_SCENE_TEMPL,
	TEMPL_TYPE_MAPELEM_CTRL_TEMPL,
	TEMPL_TYPE_CLIENT_MAP_GLOBAL,

	TEMPL_TYPE_MAX
};

const int32_t kNormalizedProbValue = 10000; // 万分数的1


/**
 * @brief check_prob_valid
 *    用于检查概率是否有效，单位都有万分数
 */
inline bool check_prob_valid(int probability)
{
	if (probability < 0 || probability > kNormalizedProbValue)
		return false;
	return true;
}


// 计算中文字符长度
#define UTF8_LEN(size) (size*4)

// 时间段使用的常量
#define TS_MAX_MONTHS_COUNT 12
#define TS_MAX_DAYS_COUNT 31

/**
 * @brief 时间段
 *    (1)start_date，end_date必须同时有值或无值
 *    (2)start_time，end_time必须同时有值或无值
 *
 * 成员变量：
 *      months：哪些月份满足要求
 *      is_day_of_month：指定days表示几号还是星期几
 *      days：哪些日子满足要求
 *      start_time/end_time：当日的时分秒处于该范围内才满足要求
 * 取值范围限制：
 *      months：size小于等于12，值的有效范围[0,11]
 *      days：当day_of_month_为true时，size小于等于31，值的有效范围[1,31]
 *            当day_of_month_为false时，size小于等于7，值的有效范围[0,6]
 *      start_time/end_time：前者小于等于后者，有效范围[0,23*3600+59*60+59]
 */
struct TimeSegment
{
	TimeSegment();

	int8_t  is_valid;    // 时间段是否生效，默认值0表示不生效，1表示生效
	int32_t start_date;  // 绝对时间，调整过的开启时间，默认值0表示不限制
	int32_t end_date;    // 绝对时间，调整过的结束时间，默认值同上
	int32_t start_time;  // 当天开启时间，记录的是秒数，默认值同上
	int32_t end_time;    // 当天结束时间，记录的是秒数，默认值同上
	int8_t  is_day_of_month; // days是不是日，默认值true表示days里保存的是日，false表示保存的是星期几
	BoundArray<int8_t, TS_MAX_DAYS_COUNT> days;     // 保存日或者星期几，空表示不限制
	BoundArray<int8_t, TS_MAX_MONTHS_COUNT> months; // 保存月份，空表示不限制

    void Pack(shared::net::ByteBuffer& buf);
    void UnPack(shared::net::ByteBuffer& buf);
};

bool CheckTimeSegment(const TimeSegment& time_seg);
void CorrectionDateTime(TimeSegment& time_seg);


/**
 * @brief 新加的template都要在base_templ.cpp里做INIT
 *    1.定义成员变量类型是注意参考templ_types.h中的typedef
 */
class BaseExtraTempl : public BasePacket
{
public:
	BaseExtraTempl(BasePacket::Type id)
		: BasePacket(id)
	{ }

	virtual ~BaseExtraTempl() { }

	TemplID    templ_id; // 模板id，全局唯一

	virtual void Marshal();
	virtual void Unmarshal();
	virtual bool CheckDataValidity();
	virtual std::string TemplateName() const = 0;

protected:
	// 子类使用以下函数
	virtual void OnMarshal() = 0;
	virtual void OnUnmarshal() = 0;
	virtual bool OnCheckDataValidity() const = 0;  // 检查模板里数据的有效性，每个子类自行定义
};


///
/// base data template manager
///
class BaseExtraTemplManager : public shared::Singleton<BaseExtraTemplManager>
{
	friend class shared::Singleton<BaseExtraTemplManager>;
public:
	static inline BaseExtraTemplManager* GetInstance() {
		return &(get_mutable_instance());
	}

	static BaseExtraTempl* CreatePacket(BaseExtraTempl::Type id);
	static bool InsertPacket(uint16_t type, BaseExtraTempl* packet);
	static bool IsValidType(int32_t type);


protected:
	BaseExtraTemplManager() { }
	~BaseExtraTemplManager() { }

	bool OnInsertPacket(uint16_t type, BaseExtraTempl* packet);
	BaseExtraTempl* OnCreatePacket(BaseExtraTempl::Type id);


private:
	typedef std::map<BaseExtraTempl::Type, BaseExtraTempl*> BaseExtraTemplMap;
	BaseExtraTemplMap packet_map_;
};

} // namespace extraTempl

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_BASE_TEMPL_H_
