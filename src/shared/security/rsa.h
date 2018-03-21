#ifndef SHARED_RSA_H
#define SHARED_RSA_H

#include "security.h"
#include <openssl/rsa.h>

namespace shared
{
namespace net
{

class RSA : public Security
{
public:
	RSA();
	virtual ~RSA();
	RSA(const RSA& rhs);
	virtual bool SetParameter(const Buffer& param);
	virtual void GetParameter(Buffer& param);
	virtual bool Update(Buffer& input);
protected:
	virtual Security* Clone() const;
private:
	Buffer param_;
    std::string crypto_type_;
	::RSA* key_;
};

} // namespace net
} // namespace shared

#endif // SHARED_RSA_H
