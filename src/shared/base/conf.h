#ifndef SHARED_BASE_CONF_H_
#define SHARED_BASE_CONF_H_

#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "shared/base/mutex.h"
#include "shared/base/copyable.h"


namespace shared {

class Conf : public copyable
{
public:
	typedef std::string section_type;
	typedef std::string key_type;
	typedef std::string value_type;
	
	Conf() : mtime_(0), is_inited_(false) { }

	Conf(const Conf& rhs) {	copy(rhs); }
	void operator=(const Conf& rhs) { copy(rhs); }

private:
	struct stringcasecmp
	{
		bool operator() (const std::string &x, const std::string &y) const { return strcasecmp(x.c_str(), y.c_str()) < 0; }
	};

	typedef std::map<key_type, value_type, stringcasecmp>       SectionHash;
	typedef std::map<section_type, SectionHash, stringcasecmp>  ConfHash;
	ConfHash       confhash_;
    std::string    filename_;
	time_t         mtime_;
	bool           is_inited_;
	MutexLock      mutex_;	

	void copy(const Conf& rhs)
	{
		confhash_  = rhs.confhash_;
		filename_  = rhs.filename_;
		mtime_     = rhs.mtime_;
		is_inited_ = rhs.is_inited_;
	}

	void reload()
	{
		struct stat st;
		MutexLockGuard lock(mutex_);
		for ( stat(filename_.c_str(), &st); mtime_ != st.st_mtime; stat(filename_.c_str(), &st) )
		{
			mtime_ = st.st_mtime;
			std::ifstream ifs(filename_.c_str());
            std::string line;
			section_type section;
			SectionHash sechash;
			if (!confhash_.empty()) confhash_.clear();
			while (std::getline(ifs, line))
			{
				const char c = line[0];
				if (c == '#' || c == ';') continue;
				if (c == '[')
				{
                    std::string::size_type start = line.find_first_not_of(" \t", 1);
					if (start == std::string::npos) continue;
                    std::string::size_type end   = line.find_first_of(" \t]", start);
					if (end   == std::string::npos) continue;
					if (!section.empty()) confhash_[section] = sechash;
					section = section_type(line, start, end - start);
					sechash.clear();
				} else {
                    std::string::size_type key_start = line.find_first_not_of(" \t");
					if (key_start == std::string::npos) continue;
                    std::string::size_type key_end   = line.find_first_of(" \t=", key_start);
					if (key_end == std::string::npos) continue;
                    std::string::size_type val_start = line.find_first_of("=", key_end);
					if (val_start == std::string::npos) continue;
					val_start = line.find_first_not_of(" \t", val_start + 1);
					if (val_start == std::string::npos) continue;
                    std::string::size_type val_end = line.find_last_not_of(" \t\r\n");
					if (val_end == std::string::npos) continue;
					if (val_end < val_start) continue;
					sechash[key_type(line, key_start, key_end - key_start)] = value_type(line, val_start,val_end - val_start + 1);
				}
			}
			if (!section.empty()) confhash_[section] = sechash;
		}
	}

	void merge(Conf& rhs)
	{
		MutexLockGuard lock(mutex_);
		ConfHash::iterator it_c = rhs.confhash_.begin();
		for (; it_c != rhs.confhash_.end(); ++it_c)
		{
			// 查看是否有一样的section
			ConfHash::iterator it_e = confhash_.find(it_c->first);
			if (it_e == confhash_.end())
			{
				// 没有一样的，直接复制
				confhash_[it_c->first] = it_c->second;
			}
			else
			{
				SectionHash& shash   = it_c->second;
				SectionHash& sechash = it_e->second;
				SectionHash::iterator it_s = shash.begin();
				for (; it_s != shash.end(); ++it_s)
				{
					sechash[it_s->first] = it_s->second;
				}
			}
		}
	}


public:
	value_type find(const section_type &section, const key_type &key)
	{
		MutexLockGuard lock(mutex_);
		return confhash_[section][key];
	}

	value_type put(const section_type &section, const key_type &key, const value_type &value)
	{
		MutexLockGuard lock(mutex_);
		value_type oldvalue = confhash_[section][key];
		confhash_[section][key] = value;
		return oldvalue;
	}

	void getkeys(const section_type &section, std::vector<key_type> &keys)
	{
		keys.clear();
		MutexLockGuard lock(mutex_);
		SectionHash h = confhash_[section];
		for( SectionHash::const_iterator it=h.begin(); it!=h.end(); ++it )
		{
			keys.push_back( (*it).first );
		}
	}

	int get_int_value(const section_type& section, const key_type& key)
	{
		MutexLockGuard lock(mutex_);
		value_type tmpvalue = confhash_[section][key];
		
		return atoi(tmpvalue.c_str());
	}

	float get_float_value(const section_type& section, const key_type& key)
	{
		MutexLockGuard lock(mutex_);
		value_type tmpvalue = confhash_[section][key];
		
		return atof(tmpvalue.c_str());
	}

	const std::string file_name() const
	{
		return filename_;
	}

	bool is_inited() const
	{
		return is_inited_;
	}

	bool Init(const char *file)
	{
		if (is_inited_) return false;

		if (file && access(file, R_OK) == 0) {
			filename_ = file;
			reload();
			is_inited_ = true;
		}
		else {
			return false;
		}

		return true; 
	}
	
	bool AppendConfFile(const char* file)
	{
		if (!is_inited_)
		{
			return Init(file);
		}
		else
		{
			Conf tmp;
			if (tmp.Init(file))
			{
				merge(tmp);
				return true;
			}
		}
		return false;
	}

	void Dump(FILE* out)
	{
		MutexLockGuard lock(mutex_);
		ConfHash::iterator it_c = confhash_.begin();
		for (; it_c != confhash_.end(); ++it_c)
		{
			fprintf(out, "[%s]\n", it_c->first.c_str());
			SectionHash& shash         = it_c->second;
			SectionHash::iterator it_s = shash.begin();
			for (; it_s != shash.end(); ++it_s)
			{
				fprintf(out, "%s\t=\t%s\n", it_s->first.c_str(), it_s->second.c_str());
			}
			fprintf(out, "\n");
		}
	}
};	

}

#endif // SHARED_BASE_CONF_H_

