/*
Copyright (c) 2021-2024 Nicolas Beddows <nicolas.beddows@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <assert.h>
#include <fstream>

#include "meen/MEEN_Error.h"
#include "meen/opt/Opt.h"

namespace MachEmu
{
	Opt::Opt()
	{
		json_ = nlohmann::json::parse(Opt::DefaultOpts(), nullptr, false);
		assert(json_.is_discarded() == false);

	}

	constexpr std::string Opt::DefaultOpts()
	{
		std::string defaults =	R"({"clockResolution":-1)"
#ifdef ENABLE_MEEN_SAVE
								R"(,"compressor":")"
#ifdef ENABLE_ZLIB
								"zlib"
#else
								"none"
#endif // ENABLE_ZLIB
								R"(","encoder":")"
#ifdef ENABLE_BASE64
								"base64"
#else
								"none"
#endif // ENABLE_BASE64
								R"(","loadAsync":false,"rom":{"file":[{"offset":0,"size":0}]},"ram":{"block":[{"offset":0,"size":0}]},"saveAsync":false)"
#endif // ENABLE_MEEN_SAVE
								R"(,"isrFreq":0,"runAsync":false})";
		return defaults;
	}

	std::error_code Opt::SetOptions(const char* opts)
	{
		nlohmann::json json;
		auto err = make_error_code(errc::no_error);

		if(opts == nullptr)
		{
			json = nlohmann::json::parse(Opt::DefaultOpts(), nullptr, false);
			assert(json.is_discarded() == false);

			if(json.is_discarded() == true)
			{
				return make_error_code(errc::json_parse);
			}
		}
		else
		{
			std::string_view jsonStr(opts);

			if (jsonStr.starts_with("file://") == true)
			{
				jsonStr.remove_prefix(strlen("file://"));
				//todo: use FILE* here, test wih nullptr, amke sure it doesn't throw
				std::ifstream fin(std::string(jsonStr.data(), jsonStr.size()));
				json = nlohmann::json::parse(fin, nullptr, false);
			}
			else
			{
				// parse as if raw json
				json = nlohmann::json::parse(std::string(jsonStr.data(), jsonStr.length()), nullptr, false);
			}

			if(json.is_discarded() == true)
			{
				return make_error_code(errc::json_parse);
			}

			if (json_.contains("cpu") == true && json.contains("cpu") == true)
			{
				return make_error_code(errc::json_config);
			}

			if (json.contains("isrFreq") == true && json["isrFreq"].get<double>() < 0)
			{
				return make_error_code(errc::json_config);
			}

#ifdef ENABLE_MEEN_SAVE
#ifndef ENABLE_ZLIB
			if (json.contains("compressor") == true && json["compressor"].get<std::string>() == "zlib")
			{
				return make_error_code(errc::no_zlib);
			}
#endif

			// Handle deprecated properties, remove in 2.0.0			
			// Only convert if we don't have ram/rom properties, if we do, then any deprecated property
			// usage will be dropped
			if (json.contains("ram") == false && json.contains("rom") == false)
			{
				auto j = nlohmann::json::parse(R"({"rom":{"file":[{"offset":0,"size":0}]},"ram":{"block":[{"offset":0,"size":0}]}})");
				bool update = false;

				if(json.contains("ramOffset"))
				{
					j["ram"]["block"][0]["offset"] = json["ramOffset"].get<uint16_t>();
					update = true;
				}

				if(json.contains("ramSize") && !json.contains("ram"))
				{
					j["ram"]["block"][0]["size"] = json["ramSize"].get<uint16_t>();
					update = true;
				}

				if(json.contains("romOffset") && !json.contains("ram"))
				{
					j["rom"]["file"][0]["offset"] = json["romOffset"].get<uint16_t>();
					update = true;
				}

				if(json.contains("ramSize"))
				{
					j["rom"]["file"][0]["size"] = json["romSize"].get<uint16_t>();
					update = true;
				}

				if (update == true)
				{
					json.update(j);
				}
			}
			// End remove
#endif // ENABLE_MEEN_SAVE
		}

		json_.update(json);

		return err;
	}

	int64_t Opt::ClockResolution() const
	{
		return json_["clockResolution"].get<int64_t>();
	}

	std::string Opt::CpuType() const
	{
		if (json_.contains("cpu") == true)
		{
			return json_["cpu"].get<std::string>();
		}
		else
		{
			return "";
		}
	}

	double Opt::ISRFreq() const
	{
		return json_["isrFreq"].get<double>();
	}

	bool Opt::RunAsync() const
	{
		return json_["runAsync"].get<bool>();
	}

#ifdef ENABLE_MEEN_SAVE
	std::string Opt::Compressor() const
	{
		return json_["compressor"].get<std::string>();
	}

	std::string Opt::Encoder() const
	{
		return json_["encoder"].get<std::string>();
	}

	bool Opt::LoadAsync() const
	{
		return json_["loadAsync"].get<bool>();
	}

	std::vector<std::pair<uint16_t, uint16_t>> Opt::Ram() const
	{
		std::vector<std::pair<uint16_t, uint16_t>> ram;
		auto blocks = json_["ram"]["block"];

		for (const auto& block : blocks)
		{
			ram.emplace_back(std::pair(block["offset"].get<uint16_t>(), block["size"].get<uint16_t>()));
		}

		return ram;
	}

	std::vector<std::pair<uint16_t, uint16_t>> Opt::Rom() const
	{
		std::vector<std::pair<uint16_t, uint16_t>> rom;
		auto files = json_["rom"]["file"];

		for(const auto& file : files)
		{
			rom.emplace_back(std::pair(file["offset"].get<uint16_t>(), file["size"].get<uint16_t>()));
		}

		return rom;
	}

	bool Opt::SaveAsync() const
	{
		return json_["saveAsync"].get<bool>();
	}
#endif // ENABLE_MEEN_SAVE
} // namespace MachEmu