#include <fstream>

#include "nlohmann/json.hpp"
#include "Opt/Opt.h"

namespace MachEmu
{
	void Opt::SetOptions(std::string_view jsonStr)
	{
		nlohmann::json json;

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

		if (json.contains("cpu") == true)
		{
			cpuType_ = json["cpu"].get<std::string_view>();
		}

		if (json.contains("runAsync") == true)
		{
			runAsync_ = json["runAsync"].get<bool>();
		}

		if (json.contains("isrFreq") == true)
		{
			isrFreq_ = json["isrFreq"].get<double>();

			if (isrFreq_ < 0)
			{
				throw std::invalid_argument("isrFreq must be >= 0");
			}
		}
	}

	std::string_view Opt::CpuType() const
	{
		return cpuType_;
	}

	double Opt::ISRFreq() const
	{
		return isrFreq_;
	}

	bool Opt::RunAsync() const
	{
		return runAsync_;
	}
} // namespace MachEmu
