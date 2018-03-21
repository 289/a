#include "auction_config.h"

#include <set>

#include "common/3rd/pugixml/inc/pugixml.hpp"
#include "dbgprt.h"


namespace gamed {

using namespace pugi;

AuctionConfig::AuctionConfig()
{
}

AuctionConfig::~AuctionConfig()
{
}

bool AuctionConfig::Init(const char* path)
{
    // 相当于所有物品都不能拍卖
    if (path == NULL)
        return true;

    xml_document doc;
    xml_parse_result result = doc.load_file(path);
    if (!result)
    {
        __PRINTF("Auction config XML load_file error! %s", path);
        return false;
    }

    std::set<Entry> check_set;

    // foreach category
    xml_node categoryNode = doc.child("Category");
    for (; categoryNode; categoryNode = categoryNode.next_sibling("Category"))
    {
        int categoryId = categoryNode.attribute("id").as_int();
        xml_node subCategoryNode = categoryNode.child("SubCategory");
        for (; subCategoryNode; subCategoryNode = subCategoryNode.next_sibling("SubCategory"))
        {
            int subCategoryId = subCategoryNode.attribute("id").as_int();
            Entry tmpent(categoryId, subCategoryId);
            xml_node itemIdNode = subCategoryNode.child("ItemId");
            for (; itemIdNode; itemIdNode = itemIdNode.next_sibling("ItemId"))
            {
                int itemId = itemIdNode.attribute("value").as_int();
                auctionable_item_map_[itemId].push_back(tmpent);
            }
            if (check_set.insert(tmpent).second == false)
            {
                __PRINTF("ERR：拍卖配置表有重复的一级二级目录id！");
                ASSERT(false);
            }
        }
    }

    return true;
}

bool AuctionConfig::IsAuctionableItem(int32_t item_id) const
{
    AuctionableItemMap::const_iterator it = auctionable_item_map_.find(item_id);
    if (it != auctionable_item_map_.end())
    {
        // found
        return true;
    }

    return false;
}

} // namespace gamed
