#ifndef GAMED_GS_GLOBAL_AUCTION_CONFIG_H_
#define GAMED_GS_GLOBAL_AUCTION_CONFIG_H_

#include <map>
#include <vector>
#include <stdint.h>

#include "shared/base/singleton.h"


namespace gamed {

class AuctionConfig : public shared::Singleton<AuctionConfig>
{
    friend class shared::Singleton<AuctionConfig>;
public:
    static inline AuctionConfig* GetInstance() {
		return &(get_mutable_instance());
	}

    bool Init(const char* path);
    bool IsAuctionableItem(int32_t item_id) const;


protected:
    AuctionConfig();
    ~AuctionConfig();


private:
    typedef std::pair<int32_t/*category_id*/, int32_t/*subcategory_id*/> Entry;
    typedef std::vector<Entry> CategoryVec;
    typedef std::map<int32_t/*item_id*/, CategoryVec> AuctionableItemMap;
    AuctionableItemMap auctionable_item_map_;
};


#define s_pAuctionCfg gamed::AuctionConfig::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_AUCTION_CONFIG_H_
