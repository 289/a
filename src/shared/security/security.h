#ifndef SHARED_SECURITY_H
#define SHARED_SECURITY_H

#include <map>
#include "shared/net/buffer.h"

namespace shared
{
namespace net
{

class Security
{
public:
	enum Type
	{
		MIN_TYPE,
		RANDOM,
		RC4,
		MD5HASH,
		SHA1HASH,
		RSA,
		AES,
		MAX_TYPE,
	};

	Security(const Security& rhs);
	virtual ~Security();
	virtual bool SetParameter(const Buffer& param);
	virtual void GetParameter(Buffer& param);
	virtual bool Update(Buffer& input) = 0;
	static Security* Create(Type type);
protected:
	Security(Type t);
	Type GetType() const;
	virtual Security* Clone() const = 0;
private:
	Type type_;
	typedef std::map<Type, const Security*> SecurityMap;
	static SecurityMap& GetSecurityMap();
};

} // namespace net
} // namespace shared

#endif // SHARED_SECURITY_H
