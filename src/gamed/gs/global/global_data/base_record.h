#ifndef GAMED_GS_GLOBAL_DATA_BASE_RECORD_H_
#define GAMED_GS_GLOBAL_DATA_BASE_RECORD_H_

#include <stdint.h>


namespace gamed {
namespace globalData {

class BaseRecord
{
public:
    BaseRecord(int type) : type_(type) {}
    virtual ~BaseRecord() {}

	int32_t GetType() const { return type_; }
	
    virtual bool SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len) = 0;
	virtual void ClearAll() = 0;

private:
    int32_t type_;
};

} // namespace globalData
} // namespace gamed

#endif // GAMED_GS_GLOBAL_DATA_BASE_RECORD_H_
