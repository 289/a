#ifndef SHARED_BASE_STRTOKEN_H_
#define SHARED_BASE_STRTOKEN_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>


namespace shared {

class StrToken
{
public:
	StrToken() { }

	template<typename T>
	int GetTokenData(const char* srcstr, const char* delim, std::vector<T>& output_vec)
	{
		if (NULL == srcstr || NULL == delim 
		 || strlen(srcstr) == 0 || strlen(delim) == 0)
		{
			return -1;
		}

		char* saveptr;
		size_t src_len = strlen(srcstr);
		char* tmpstr   = static_cast<char*>(malloc(src_len+1));
		memset(tmpstr, 0, src_len+1);
		strncpy(tmpstr, srcstr, src_len);
		char* tok = strtok_r(tmpstr, delim, &saveptr);
		while (NULL != tok)
		{
			T content;
			if (GetTypeTContent<T>(tok, content)) 
			{
				free(tmpstr);
				return -1;
			}

			output_vec.push_back(content);
			tok = strtok_r(NULL, delim, &saveptr);
		}

		free(tmpstr);
		return 0;
	}

private:
	template<typename T>
	inline int GetTypeTContent(char* tok, T& content) {
		printf("Error in StrToken::GetTypeTContent(), No special type correspondence\n");
		return -1;
	}
};

///
/// functions
///
template<>
inline int StrToken::GetTypeTContent<int>(char* tok, int& content)
{
	content = atoi(tok);
	return 0;
}

template<>
inline int StrToken::GetTypeTContent<float>(char* tok, float& content)
{
	content = static_cast<float>(atof(tok));
	return 0;
}

template<>
inline int StrToken::GetTypeTContent<double>(char* tok, double& content)
{
	content = atof(tok);
	return 0;
}

template<>
inline int StrToken::GetTypeTContent<std::string>(char* tok, std::string& content)
{
	content = tok;
	return 0;
}

} // namespace shared

#endif // SHARED_BASE_STRTOKEN_H_
