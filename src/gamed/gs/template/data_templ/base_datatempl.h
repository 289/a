#ifndef GAMED_GS_TEMPLATE_DATATEMPL_BASE_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_BASE_TEMPL_H_

#include "shared/base/singleton.h"
#include "shared/net/packet/bit_set.h"

#include "templ_util.h"
#include "templ_types.h"


namespace dataTempl {

using namespace shared::net;

enum DATA_TEMPLATE_TYPE
{
	TEMPL_TYPE_INVALID = 0,

// 1
	TEMPL_TYPE_PLAYER_CLASS_TEMPL, // 玩家职业模板
	TEMPL_TYPE_MONSTER_TEMPL,      // 大世界中怪物模板
	TEMPL_TYPE_SERVICE_NPC_TEMPL,  // 大世界中服务NPC模板
	TEMPL_TYPE_GLOBAL_CONFIG,      // 全局配置表（全局唯一）

// 5
	TEMPL_TYPE_EQUIP,              // 装备模板
    TEMPL_TYPE_CARD,               // 卡牌模板
	TEMPL_TYPE_REFINE_CONFIG,      // 装备精练表模板
	TEMPL_TYPE_MONSTER_DROP_TEMPL, // 怪物掉落模板
	TEMPL_TYPE_NORMAL_ITEM,        // 简单物品

// 10
    TEMPL_TYPE_PET,                // 宠物模板
	TEMPL_TYPE_GOLEM,              // 魔偶模板
	TEMPL_TYPE_GLOBAL_DROP,        // 全局掉落
	TEMPL_TYPE_INSTANCE_TEMPL,     // 副本模板
	TEMPL_TYPE_SKILL_TREE,         // 技能树

// 15
	TEMPL_TYPE_HIDDEN_ITEM,        // 隐藏物品
	TEMPL_TYPE_TASK_ITEM,          // 任务物品
	TEMPL_TYPE_MINE_TEMPL,         // 矿物模板
	TEMPL_TYPE_MALL,               // 商城模板
	TEMPL_TYPE_MALL_CLASS,         // 商城分类模板

// 20
	TEMPL_TYPE_SHOP,               // 商店模板
	TEMPL_TYPE_FILL_BLANK_CFG_TBL, // 连线游戏配置表
	TEMPL_TYPE_GLOBAL_COUNTER,     // 全局计数器
	TEMPL_TYPE_INSTANCE_GROUP,     // 副本组模板，用于客户端显示
	TEMPL_TYPE_TASK_SCROLL,        // 物品道具：任务卷轴

// 25
	TEMPL_TYPE_PET_ITEM,           // 宠物物品模板
	TEMPL_TYPE_TALENT,             // 天赋模板
	TEMPL_TYPE_TALENT_GROUP,       // 天赋组模板
    TEMPL_TYPE_SKILL_ITEM,         // 技能物品模板
    TEMPL_TYPE_TITLE,              // 称号模板

// 30
    TEMPL_TYPE_REPUTATION,         // 声望模板
    TEMPL_TYPE_TALENT_ITEM,        // 天赋物品模板
    TEMPL_TYPE_BATTLEGROUND,       // 战场模板
    TEMPL_TYPE_ENHANCE,            // 附魔模板
    TEMPL_TYPE_ENHANCE_GROUP,      // 附魔库模板

// 35
    TEMPL_TYPE_COOLDOWN_GROUP,     // 冷却组
    TEMPL_TYPE_STAR,               // 星盘
    TEMPL_TYPE_PLAYER_COUNTER,     // 玩家计数器
    TEMPL_TYPE_GEVENT_GROUP,       // 活动组
    TEMPL_TYPE_GEVENT,             // 活动

// 40
    TEMPL_TYPE_MOUNT,               // 坐骑
    TEMPL_TYPE_MOUNT_EQUIP,         // 骑具
    TEMPL_TYPE_WORLDBOSS_AWARD,     // 世界BOSS奖励
    TEMPL_TYPE_GIFT_BAG,            // 礼包模板
    TEMPL_TYPE_BOSS_CHALLENGE,      // 界面BOSS挑战

// 45
    TEMPL_TYPE_INFO_PROMPT,         // 消息提示


	TEMPL_TYPE_MAX
};

enum EQUIP_TYPE
{
	EQUIP_TYPE_INVALID,
	EQUIP_TYPE_WEAPON,
	EQUIP_TYPE_ARMOR,
	EQUIP_TYPE_MAX,
};

enum GENDER_TYPE
{
	GT_MALE   = 0,
	GT_FEMALE = 1,
};

// 需要与player/player_def.h里一致
typedef BitSet<128> PlayerClsMask;

// 计算中文字符长度
#define UTF8_LEN(size) (size*4)

// 时间段使用的常量
#define TS_MAX_MONTHS_COUNT 12
#define TS_MAX_DAYS_COUNT 31

#ifndef MAX_PROP_NUM
#define MAX_PROP_NUM 16 // 最大属性个数
#endif


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
 * @brief 坐标
 */
struct Coordinate
{
    float x;
    float y;
    NESTED_DEFINE(x, y);
};


/**
 * @brief 物品的冷却数据
 *  （1）有冷却的物品，使用该结构保存冷却数据
 *  （2）参考skill_item_templ，变量定义为cooldown_data，否则
 *       CheckItemCDGroup函数编译错误
 */
struct ItemCoolDownData
{
    bool is_valid() const  
    {
        if (cd_group_id < 0 || cd_time < 0)
            return false;
        return true;
    }

    int32_t cd_group_id; // 冷却组id
    int32_t cd_time;     // 物品自身的冷却时间
    NESTED_DEFINE(cd_group_id, cd_time);
};


/**
 * @brief 新加的template都要在base_datatempl.cpp里做INIT
 *    1.定义成员变量类型是注意参考templ_types.h中的typedef
 */
class BaseDataTempl : public BasePacket
{
public:
	BaseDataTempl(BasePacket::Type type)
		: BasePacket(type)
	{ }

	virtual ~BaseDataTempl() { }

	TemplID    templ_id; // 模板id，全局唯一

	virtual void Marshal();
	virtual void Unmarshal();
	virtual bool CheckDataValidity();

	inline void set_templ_id(TemplID id) { templ_id = id; }

protected:
	// 子类使用以下函数
	virtual void OnMarshal() = 0;
	virtual void OnUnmarshal() = 0;
	virtual bool OnCheckDataValidity() const = 0;  // 检查模板里数据的有效性，每个子类自行定义
};


/**
 * @brief 物品模板的基类
 *  （1）注意：继承该类的子类都要在CollectAllItemTempl()里添加代码
 */
class ItemDataTempl : public BaseDataTempl
{
public:
    static const int kMaxVisibleNameLen = UTF8_LEN(12);  // 最多支持12个中文字符
	static const int kMaxVisibleDespLen = UTF8_LEN(128); // 最多支持128个中文字符
	static const int kMaxPicPathLen     = 512;

	ItemDataTempl(BasePacket::Type type)
		: BaseDataTempl(type)
	{ }

	virtual ~ItemDataTempl()
	{ }

    // TODO 注意：继承该类的子类都要在CollectAllItemTempl()里添加代码

	int32_t pile_limit;    //堆叠上限
	int32_t proc_type;     //处理规则
	int32_t recycle_price; //卖店价
	int8_t  quality;       //物品品质
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;   //物品名称
	BoundArray<uint8_t, kMaxVisibleDespLen> visible_desp;   //物品描述
	BoundArray<uint8_t, kMaxPicPathLen>     icon_file_path; //物品图标文件路径

	virtual void Marshal();
	virtual void Unmarshal();
	virtual bool CheckDataValidity();
};


///
/// base data template manager
///
class BaseDataTemplManager : public shared::Singleton<BaseDataTemplManager>
{
	friend class shared::Singleton<BaseDataTemplManager>;
public:
	static inline BaseDataTemplManager* GetInstance() {
		return &(get_mutable_instance());
	}

	static BaseDataTempl* CreatePacket(BaseDataTempl::Type id);
	static bool InsertPacket(uint16_t type, BaseDataTempl* packet);
	static bool IsValidType(int32_t type);


protected:
	BaseDataTemplManager() { }
	~BaseDataTemplManager() { }

	bool OnInsertPacket(uint16_t type, BaseDataTempl* packet);
	BaseDataTempl* OnCreatePacket(BaseDataTempl::Type id);


private:
	typedef std::map<BaseDataTempl::Type, BaseDataTempl*> BaseDataTemplMap;
	BaseDataTemplMap packet_map_;
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_BASE_TEMPL_H_
