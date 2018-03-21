#ifndef SHARED_MD5_H
#define SHARED_MD5_H

#include "security.h"

namespace shared
{
namespace net
{

class MD5Hash : public Security
{
public:
	MD5Hash();
	~MD5Hash();
	MD5Hash(const MD5Hash& rhs);
	virtual bool Update(Buffer& input);
	bool PrivateUpdate(Buffer& input);
protected:
	virtual Security* Clone() const;
};

} // namespace net
} // namespace shared

#endif // SHARED_MD5_H
