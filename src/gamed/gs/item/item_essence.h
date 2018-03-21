#ifndef __GAMED_GS_ITEM_ITEM_ESSENCE_H__
#define __GAMED_GS_ITEM_ITEM_ESSENCE_H__

/**********************************************************************************
 * @brief: 物品动态属性定义
 *
 * @brief: 1) 支持插入/删除/更新动态属性；
 *         2) 禁止修改动态属性长度，改变将导致解析DB端物品存盘数据错误
 *         3) 为了将来可能的扩展，定义动态属性结构体时建议预留几个字节，
 *            将来使用预留字段时，可以拆分使用，只要保证总长度不变既可
 *
 *
 *  @code for item-content
 *  +-----------+------+------+--------+------+
 *	| ess_count | ESS1 | ESS2 | ...... | ESSN |
 *	+-----------+------+------+--------+------+
 *	ess_count 表示后面ess的个数,类型为uint32_t
 *	item-content内部结构，上面各字段长度和即item-length
 *
 *
 *	@code for specfic essence
 *	+--------+------------+
 *	| specfic-ess-content |
 *	+--------+------------+
 *	| ess-id | ess-content|
 *	+--------+------------+
 *	ess-content的前4个字节为ess-id,类型为uint32_t
 *	ess-id 表示当前ess的类型，解析时根据ID来选择相应的ess结构
 *
**********************************************************************************************/

#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "item_ess_def.h"

namespace gamed
{

/**
 * @class ItemEssence
 * @brief 物品动态属性管理器
 */
class ItemEssence
{
	struct EssFinder
	{
		ESSENCE_ID id;
		EssFinder(ESSENCE_ID _id): id(_id) {}
		bool operator() (const EssBase* pEss)
		{
			return pEss->id == id;
		}
	};
public:
	ItemEssence();
	~ItemEssence();

    ItemEssence& operator=(const ItemEssence& rhs);

    void LoadScalableEss(const std::string& content);
    void SaveScalableEss(std::string& content) const;
	void LoadEss(const std::string& content);
	void SaveEss(std::string& content) const;
	void InsertEss(EssBase* pEss);
	void DeleteEss(ESSENCE_ID ess_id);
    void Clear();

	template<typename T>
	T* QueryEss(ESSENCE_ID ess_id)
	{
		EssVector::iterator it = std::find_if(ess_vec_.begin(), ess_vec_.end(), EssFinder(ess_id));
        return it == ess_vec_.end() ? NULL : static_cast<T*>(*it);
	}

	template<typename T>
	const T* QueryEss(ESSENCE_ID ess_id) const
	{
		EssVector::const_iterator it = std::find_if(ess_vec_.begin(), ess_vec_.end(), EssFinder(ess_id));
        return it == ess_vec_.end() ? NULL : static_cast<const T*>(*it);
	}
private:
    void Pack(shared::net::ByteBuffer& buf) const;
    void UnPack(shared::net::ByteBuffer& buf);
private:
	typedef std::vector<EssBase*> EssVector;
	EssVector ess_vec_;
};

} // namespace gamed

#endif
