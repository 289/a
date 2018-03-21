#ifndef TASK_RATIO_TABLE_TEMPL_H_
#define TASK_RATIO_TABLE_TEMPL_H_

#include "base_task_templ.h"

namespace task
{

class RatioTableTempl : public BaseTempl
{
	DECLARE_SYS_TEMPL(RatioTableTempl, TEMPL_TYPE_RATIO_TABLE);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;

	int32_t GetValue(int32_t level) const;
public:
	typedef std::map<int32_t, int32_t> RatioMap;
	RatioMap ratio_table;
};

} // namespace task

#endif // TASK_RATIO_TABLE_TEMPL_H_
