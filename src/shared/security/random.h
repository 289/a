#ifndef SHARED_RANDOM_H
#define SHARED_RANDOM_H

#include "security.h"

namespace shared
{
namespace net
{

class Random : public Security
{
public:
	Random();
	Random(const Random& rhs);
	virtual bool SetParameter(const Buffer& param);
	virtual void GetParameter(Buffer& param);
	virtual bool Update(Buffer& input);
protected:
	virtual Security* Clone() const;
private:
	Buffer param_;
};

} // namespace net
} // namespace shared

#endif // SHARED_RANDOM_H
