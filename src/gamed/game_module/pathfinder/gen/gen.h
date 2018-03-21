#include <stdio.h>
#include <string>
#include <sys/types.h>

#ifdef PLATFORM_WINDOWS

#else
#include <dirent.h>
#include <regex.h>
#endif

#include "generator.h"

using namespace pathfinder;

const char* cur_dir = ".";
const char* up_dir  = "..";

void GenerateData(const char* input_path, const char* output_path);
void GenerateMoveMapData(const std::vector<std::string>& filelist, const char* input_path, const char* output_path);
void GenerateTransMapData(const char* input_path, const char* output_path);

static bool ScanFiles(const char* input_path, std::vector<std::string>& filelist)
{

#ifdef PLATFORM_WINDOWS

#else

	DIR* dir = opendir(input_path);
	if (NULL == dir)
		return false;

	struct dirent entry;
	struct dirent* result = NULL;

	regex_t reg;
	const char* name_pattern = "^movemap_[1-9][0-9]{0,}\\.gbd$";
	regcomp(&reg, name_pattern, REG_EXTENDED);

	// read all movemap-files
	for (;;)
	{
		int rst = readdir_r(dir, &entry, &result);
		if (rst == 0 && result != NULL)
		{
			std::string __filename(entry.d_name);
			if (__filename == std::string(cur_dir))
				continue;
			if (__filename == std::string(up_dir))
				continue;

			size_t nmatch = 1;
			regmatch_t pmatch[1];
			int rst = regexec(&reg, __filename.c_str(), nmatch, pmatch, 0);
			if (rst == REG_NOMATCH)
				continue;

			filelist.push_back(__filename);
		}
		else if (result == NULL)
		{
			//reach the end of dir-stream
			break;
		}
		else if (rst != 0)
		{
			//error
			regfree(&reg);
			closedir(dir);
			return false;
		}
	}

	regfree(&reg);
	closedir(dir);

#endif

	return true;
}

void GenerateData(const char* input_path, const char* output_path)
{
	if (!input_path || !output_path)
		return;

	std::vector<std::string> __filelist;
	if (ScanFiles(input_path, __filelist) && !__filelist.empty())
		GenerateMoveMapData(__filelist, input_path, output_path);

	GenerateTransMapData(input_path, output_path);
}


void GenerateMoveMapData(const std::vector<std::string>& filelist, const char* input_path, const char* output_path)
{
	// preprocess and save movemap-data
	for (size_t i = 0; i < filelist.size(); ++ i)
	{
		std::string __src_file;
		std::string __dest_file;

		__src_file.append(input_path);
		__src_file.append(filelist[i]);

		__dest_file.append(output_path);
		__dest_file.append(filelist[i]);

		MapDataGenerator __generator;
		if (__generator.Load(__src_file.c_str()))
		{
			__generator.Process();
			__generator.Save(__dest_file.c_str());

			fprintf(stdout, "gen\t--->>>\t%s\n", __dest_file.c_str());
		}
	}
}

void GenerateTransMapData(const char* input_path, const char* output_path)
{
	std::string __src_file;
	__src_file.append(input_path);
	__src_file.append("global_trans_table.gbd");

	std::string __dest_file;
	__dest_file.append(output_path);
	__dest_file.append("global_trans_table.gbd");

	GlobalTransDataGenerator __generator;
	if (__generator.Load(__src_file.c_str()))
	{
		__generator.Process(output_path);
		__generator.Save(__dest_file.c_str());

		fprintf(stdout, "gen\t--->>>\t%s\n", __dest_file.c_str());
	}
}
