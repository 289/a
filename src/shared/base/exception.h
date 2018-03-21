#ifndef SHARED_BASE_EXCEPTION_H_
#define SHARED_BASE_EXCEPTION_H_

#include <exception>
#include "shared/base/types.h"

namespace shared {

class Exception : public std::exception
{
public:
	explicit Exception(const char* what);
	explicit Exception(const std::string& what);
	virtual ~Exception() throw();
	virtual const char* What() const throw();
	const char* StackTrace() const throw();

private:
	void FillStackTrace();

    std::string message_;
    std::string stack_;
};

}

#endif // SHARED_BASE_EXCEPTION_H_
