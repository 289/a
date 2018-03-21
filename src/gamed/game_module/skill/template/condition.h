#ifndef SKILL_DATATEMPL_CONDITION_H_
#define SKILL_DATATEMPL_CONDITION_H_

#include "skill_types.h"

namespace skill
{

class Condition
{
public:
	Condition()
		: type(COND_PROP), op(OP_GREATER), id(0), value("0")
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	int8_t op;
	int32_t id;		// COND_PROP：属性ID，COND_ITEM：物品ID
	std::string value;
	NESTED_DEFINE(type, op, id, value);
};

inline bool Condition::CheckDataValidity() const
{
	CHECK_INRANGE(type, COND_PROP, COND_ITEM)
	CHECK_INRANGE(op, OP_LESS, OP_GREATER_EQUAL)
	if (type == COND_PROP)
	{
		CHECK_INRANGE(id, PROP_INDEX_MAX_HP, PROP_INDEX_CON2)
	}
	return true;
}
typedef std::vector<Condition> CondVec;

} // namespace skill

#endif // SKILL_DATATEMPL_CONDITION_H_
