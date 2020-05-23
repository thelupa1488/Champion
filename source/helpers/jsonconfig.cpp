#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "jsonconfig.h"


#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace config
{
	const std::string path = "C:/Champion";
	std::map<std::string, std::map<std::string, Json::Value>> cached;

	std::string path_to_folder(const std::string& folder)
	{
		return path + (folder.empty() ? "" : "/" + folder);
	}

	std::string path_to_file(const std::string& name, const std::string& folder)
	{
		const auto next = path_to_folder(folder);

		return next + (name.empty() ? "" : "/" + name);
	}

	Json::Value get_source(const std::string& name, const std::string& folder)
	{
		Json::Value root;

		std::ifstream source(path_to_file(name, folder), std::ifstream::binary);
		if (!source.good())
		{
#ifdef _DEBUG
			//console::print("file %s doesnt exists", name.c_str());
#endif
			return root;
		}

		source >> root;

#ifdef _DEBUG
		//console::print("readed %s file", name.c_str());
#endif

		return root;
	}

	Json::Value get_and_cache_source(const std::string& name, const std::string& folder)
	{
		if (cached[folder].count(name) == 0)
			cached[folder][name] = get_source(name, folder);

		return cached[folder][name];
	}

	void cache(const std::string& folder)
	{
		const auto items = get_files(folder);
		for (auto& file : items)
			get_and_cache_source(file, folder);
	}

	std::vector<std::string> get_files(const std::string& folder)
	{
		std::vector<std::string> items;

		std::string path = path_to_folder(folder);
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
			items.emplace_back(p.path().string().substr(path.length() + 1));

		return items;
	}

	void load(const std::string& name, const std::string& folder, const bool& cache, const std::function<void(Json::Value)>& callback)
	{
		std::string path("C:\\Champion");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		std::string others("C:\\Champion\\others");
		if (!fs::is_directory(others))
			fs::create_directories(others);

		if (name.empty())
			return;

		const auto source = cache ? get_and_cache_source(name, folder) : get_source(name, folder);

		callback(source);
	}

	bool save(const std::string& name, const std::string& folder, const bool& cache, const std::function<Json::Value(void)>& callback)
	{
		std::string path("C:\\Champion");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		std::string others("C:\\Champion\\others");
		if (!fs::is_directory(others))
			fs::create_directories(others);

		if (name.empty())
			return false;

		const auto root = callback();
		if (cache)
			cached[folder][name] = root;

		Json::StyledWriter styledWriter;

		try
		{
			std::ofstream(path_to_file(name, folder)) << styledWriter.write(root);

#ifdef _DEBUG
			//console::print("%s successfully saved", name.c_str());
#endif // _DEBUG

			return true;
		}
		catch (const std::exception & ex)
		{
#ifdef _DEBUG
			//console::print("fail to save %s: %s", name.c_str(), ex.what());
#endif // _DEBUG
		}

		return false;
	}

	bool remove(const std::string& name, const std::string& folder)
	{
		const auto state = fs::remove(path_to_file(name, folder));
		if (state)
		{
			cached[folder].clear();
			cache(folder);
		}

		return state;
	}
}


namespace Config
{
	std::string GetPath(const std::string& name, const std::string& folder)
	{
		return "Champion/" + (folder.empty() ? "" : folder + "/") + name;
	}

	std::vector<std::string> Configs(const std::string& folder)
	{
		std::vector<std::string> items = { };

		std::string path("memesense/" + folder);
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
			items.emplace_back(p.path().string().substr(path.length() + 1));

		return items;
	}

	bool Delete(const std::string& name, const std::string& folder)
	{
		return fs::remove(GetPath(name, folder));
	}

	void Load(const std::string& name, const std::string& folder, std::function<void(Json::Value)> f)
	{
		if (name.empty())
			return;

		Json::Value root;

		try
		{
			std::ifstream f(GetPath(name, folder), std::ifstream::binary);
			if (!f.good())
			{
#ifdef _DEBUG
				//console::print("file doesnt exists");
#endif
				return;
			}

			f >> root;
		}
		catch (const std::exception & ex)
		{
#ifdef _DEBUG
			//console::print("Fail to load %s: %s", name.c_str(), ex.what());
#endif
			return;
		}

		f(root);

#ifdef _DEBUG
		//console::print("%s successfully loaded", name.c_str());
#endif
	}

	void Save(const std::string& name, const std::string& folder, std::function<Json::Value(void)> f)
	{
		if (name.empty())
			return;

		const auto root = f();

		Json::StyledWriter styledWriter;

		try
		{
			std::ofstream(GetPath(name, folder)) << styledWriter.write(root);

#ifdef _DEBUG
			//console::print("%s successfully saved", name.c_str());
#endif
		}
		catch (const std::exception & ex)
		{
#ifdef _DEBUG
			//console::print("Fail to save %s: %s", name.c_str(), ex.what());
#endif
		}
	}
};

namespace Option
{
	void Save(Json::Value& config, ImVec4 setting)
	{
		config["x"] = setting.x;
		config["y"] = setting.y;
		config["z"] = setting.z;
		config["w"] = setting.w;
	}

	void Save(Json::Value& config, float* setting)
	{
		config["r"] = setting[0];
		config["g"] = setting[1];
		config["b"] = setting[2];
		config["a"] = setting[3];
	}

	void Save(Json::Value& config, Color setting)
	{
		config["r"] = setting.r();
		config["g"] = setting.g();
		config["b"] = setting.b();
		config["a"] = setting.a();
	}

	void Save(Json::Value& config, char* setting, int size)
	{
		if (setting[0])
			for (int i = 0; i < size; i++)
				config[i] = setting[i];
	}

	void Load(Json::Value config, ImVec4& setting, const ImVec4& def)
	{
		if (config.isNull())
			setting = def;
		else
			setting = ImVec4(config["x"].asFloat(), config["y"].asFloat(), config["z"].asFloat(), config["w"].asFloat());
	}

	void Load(Json::Value config, Color& setting, const Color& def)
	{
		if (config.isNull())
			setting = def;
		else
			setting.SetColor(config["r"].asInt(), config["g"].asInt(), config["b"].asInt(), config["a"].asInt());
	}

	void Load(Json::Value config, int& setting, const int& def)
	{
		setting = config.isNull() ? def : config.asInt();
	}

	void Load(Json::Value config, bool& setting, const bool& def)
	{
		setting = config.isNull() ? def : config.asBool();
	}

	void Load(Json::Value config, float& setting, const float& def)
	{
		setting = config.isNull() ? def : config.asFloat();
	}
	void Load(Json::Value config, float* setting, const float& def)
	{
		if (config.isNull())
		{
			setting[0] = def;
			setting[1] = def;
			setting[2] = def;
			setting[3] = def;
		}
		else
		{
			setting[0] = config["r"].asFloat();
			setting[1] = config["g"].asFloat();
			setting[2] = config["b"].asFloat();
			setting[3] = config["a"].asFloat();
		}
	}
}