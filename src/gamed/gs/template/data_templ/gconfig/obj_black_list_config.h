#ifndef GAMED_GS_TEMPLATE_DATATEMPL_OBJ_BLACK_LIST_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_OBJ_BLACK_LIST_CONFIG_H_


namespace dataTempl {

/**
 * @brief 对象黑名单配置表（服务Npc或者矿的黑名单）
 */
class ObjBlackListConfig
{
public:
	static const size_t kMaxBlackListCount = 255;

	std::set<int32_t> templ_id_set;
	NESTED_DEFINE(templ_id_set);

	bool CheckDataValidity() const
	{
		if (templ_id_set.size() > kMaxBlackListCount)
			return false;
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_OBJ_BLACK_LIST_CONFIG_H_
