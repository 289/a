#ifndef SHARED_RC4_H
#define SHARED_RC4_H

#include "security.h"
#include <openssl/rc4.h>

namespace shared
{
namespace net
{

class RC4 : public Security
{
public:
	RC4();
	~RC4();
	RC4(const RC4& rhs);
	virtual bool SetParameter(const Buffer& param);
	virtual void GetParameter(Buffer& param);
	virtual bool Update(Buffer& input);
protected:
	virtual Security* Clone() const;
private:
	Buffer param_;
	RC4_KEY key_;
};

} // namespace net
} // namespace shared

#endif // SHARED_RC4_H
