#include <fstream>

#include "nlohmann/json.hpp"
#include "Opt/Opt.h"

namespace MachEmu
{
	Opt::Opt()
	{
		json_ = new nlohmann::json();

		if(json_ == nullptr)
		{
			throw std::bad_alloc();
		}

		std::string defaults = R"({"isrFreq":0,"runAsync":false})";
		*json_ = nlohmann::json::parse(defaults);
	}

	Opt::~Opt()
	{
		if (json_ != nullptr)
		{
			delete json_;
			json_ = nullptr;
		}
	}

	ErrorCode Opt::SetOptions(const char* opts)
	{
		std::string_view jsonStr(opts);
		nlohmann::json json;
		auto err = ErrorCode::NoError;

		if (jsonStr.starts_with("file://") == true)
		{
			jsonStr.remove_prefix(strlen("file://"));
			std::ifstream fin(std::string(jsonStr.data(), jsonStr.size()));
			json = nlohmann::json::parse(fin);
		}
		else
		{
			// parse as if raw json
			json = nlohmann::json::parse(std::string(jsonStr.data(), jsonStr.length()));
		}

		if (json_->contains("cpu") == true && json.contains("cpu") == true)
		{
			throw std::runtime_error("cpu type has already been set");
		}

#ifndef _WINDOWS
		if (json.contains("runAsync") == true && json["runAsync"].get<bool>() == true)
		{
			json["runAsync"] = false;
			err = ErrorCode::NotImplemented;
		}
#endif

		json_->update(json);

		return err;
	}

	std::string Opt::CpuType() const
	{
		if (json_->contains("cpu") == true)
		{
			return (*json_)["cpu"].get<std::string>();
		}
		else
		{
			return "";
		}
	}

	double Opt::ISRFreq() const
	{
		auto value = (*json_)["isrFreq"].get<double>();

		if (value < 0)
		{
			throw std::invalid_argument("isrFreq must be >= 0");
		}

		return value;
	}

	bool Opt::RunAsync() const
	{
		return (*json_)["runAsync"].get<bool>();
	}
} // namespace MachEmu
