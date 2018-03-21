#ifndef SHARED_AES_H
#define SHARED_AES_H

#include "security.h"
#include <openssl/aes.h>

namespace shared
{
namespace net
{

// 128位AES加密算法，采用CBC加密模式
class AES : public Security
{
public:
	AES();
	~AES();
	AES(const AES& rhs);
	// param加解密参数，要求长度为33字节
	// 第一个字节：e加密，d解密
	// 后续16字节为密钥
	// 最后16字节为初始向量IV
	virtual bool SetParameter(const Buffer& param);
	virtual void GetParameter(Buffer& param);
	// 要求input.ReadableSize()必须为16的整数倍
	virtual bool Update(Buffer& input);
protected:
	virtual Security* Clone() const;
private:
	Buffer iv_;
	bool encrypt_;
	AES_KEY key_;
};

} // namespace net
} // namespace shared

#endif // SHARED_AES_H
