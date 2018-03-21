#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_SKILL_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_SKILL_H__

#include "gs/item/item.h"

namespace dataTempl {
    class SkillItemTempl;
    class CoolDownGroupTempl;
} // namespace dataTempl

namespace gamed
{

/**
 * @class ItemSkill
 * @brief 技能物品实体
 */
class ItemSkill : public ItemBody
{
private:
    const dataTempl::SkillItemTempl* pSkill;
    const dataTempl::CoolDownGroupTempl* pCD;
public:
	explicit ItemSkill(int id): ItemBody(id), pSkill(NULL), pCD(NULL)
	{ }
	virtual ~ItemSkill()
	{ }

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual bool TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual Item::ITEM_USE OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
};

}; // namespace gamed

#endif // __GAMED_GS_ITEM_ITEM_BODY_ITEM_SKILL_H__
