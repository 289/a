#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_STAR_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_STAR_CONFIG_H_

namespace dataTempl {

/**
 * 星盘全局信息配置表
 */
struct StarInfo
{
	static const int kMaxPicPathLen     = 512;

    StarInfo()
        : id(0), star_x(0), star_y(0), line_x(0), line_y(0)
    {
    }

    int32_t id;
    int32_t star_x;
    int32_t star_y;
    int32_t line_x;
    int32_t line_y;
	BoundArray<uint8_t, kMaxPicPathLen> line_path; //连线光效

    NESTED_DEFINE(id, star_x, star_y, line_x, line_y, line_path);

    bool CheckDataValidity() const
    {
        if (id < 0)
        {
            return false;
        }
        if (star_x < 0 || star_x > 1280 || line_x < 0 || line_x > 1280)
        {
            return false;
        }
        return star_y >= 0 && line_y >= 0;
    }
};
typedef std::vector<StarInfo> StarList;

class StarConfig
{
public:
    StarList star_list;

    NESTED_DEFINE(star_list);

    bool CheckDataValidity() const
    {
        StarList::const_iterator sit = star_list.begin();
        for (; sit != star_list.end(); ++sit)
        {
            if (!sit->CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_STAR_CONFIG_H_
