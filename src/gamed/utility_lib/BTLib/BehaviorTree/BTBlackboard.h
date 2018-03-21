#ifndef GAMED_UTILITY_LIB_BTLIB_BTBLACKBOARD_H_
#define GAMED_UTILITY_LIB_BTLIB_BTBLACKBOARD_H_

#include "Copyable.h"
#include "NonCopyable.h"
#include "BTUtility.h"


namespace BTLib {

class BTParam : copyable
{
public:
	BTParam() { }
	virtual ~BTParam() { }
};

class BTBlackboard : noncopyable
{
public:
	BTBlackboard()
		: data_(NULL)
	{ }
	virtual ~BTBlackboard()
	{
		BT_SAFE_DELETE(data_);
	}

	template <typename T>
	void setBlackboard(const T& data);
	
	template <typename T>
	bool getBlackboard(T& data);

	inline void reset();
	
protected:
	BTParam* data_;
};

///
/// inline func
///
inline void BTBlackboard::reset() 
{ 
	BT_SAFE_DELETE(data_); 
}

///
/// template func
///
template <typename T>
void BTBlackboard::setBlackboard(const T& data)
{
	// delete old data, if has
	reset();

	// set new data
	T* tmpdata = new T();
	*tmpdata   = data;
	data_      = static_cast<BTParam*>(tmpdata);
}

template <typename T>
bool BTBlackboard::getBlackboard(T& data)
{
	if (data_ != NULL)
	{
		try
		{
			T& tmpdata = dynamic_cast<T&>(*data_);
			data = tmpdata;
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	return false;
}

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTBLACKBOARD_H_
