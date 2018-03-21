#ifndef GAMED_GS_EVENT_EVENTSYS_SRC_DEF_H_
#define GAMED_GS_EVENT_EVENTSYS_SRC_DEF_H_


namespace gamed {
namespace eventsys {

///
/// ErrorCode
///
enum
{
	ES_SUCCESS = 0,
	ES_FAILURE,
};

///
/// Relational Operators
///
enum
{
	RO_GREATER_THAN = 0,      // 大于 ">"   
	RO_LESS_THAN,             // 小于 "<"
	RO_EQUAL,                 // 等于 "=="
	RO_GREATER_THAN_OR_EQUAL, // 大于等于 ">="
	RO_LESS_THAN_OR_EQUAL,    // 小于等于 "<="
	RO_NOT_EQUAL,             // 不等于 "!="
};

} // namespace eventsys
} // namespace gamed

#endif // GAMED_GS_EVENT_EVENTSYS_SRC_DEF_H_
