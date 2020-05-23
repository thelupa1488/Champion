#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include "../imgui/imgui.h"
#include "../valve_sdk/misc/Color.hpp"
#include "../jsoncpp/json.h"

namespace config
{
	extern std::map<std::string, std::map<std::string, Json::Value>> cached;

	void load(const std::string& name, const std::string& folder, const bool& cache, const std::function<void(Json::Value)>& callback);
	bool save(const std::string& name, const std::string& folder, const bool& cache, const std::function<Json::Value(void)>& callback);
	bool remove(const std::string& name, const std::string& folder);

	void cache(const std::string& folder);
	std::vector<std::string> get_files(const std::string& folder);
}

namespace Option
{
	void Save(Json::Value&, ImVec4);
	void Save(Json::Value&, Color);
	void Save(Json::Value&, float*);

	void Load(Json::Value, Color&, const Color & = Color::White);
	void Load(Json::Value, int&, const int& = 0);
	void Load(Json::Value, bool&, const bool& = false);
	void Load(Json::Value, float&, const float& = 0.f);
	void Load(Json::Value config, float* setting, const float& = 1.f );
}
