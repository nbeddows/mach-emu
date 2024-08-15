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
#include <cinttypes>
#include <nlohmann/json.hpp>

#include "meen/MEEN_Error.h"
#include "meen/clock/CpuClockFactory.h"
#include "meen/cpu/CpuFactory.h"
#include "meen/machine/Machine.h"
#include "meen/utils/Utils.h"

using namespace std::chrono;

namespace MachEmu
{
	Machine::Machine(const char* options)
	{
		auto errc = SetOptions(options);

		if (errc)
		{
			printf("Machine::Machine SetOptions failed (using defaults): %s\n", errc.message().c_str());
		}

		// if no cpu type specified, set the default
		if(opt_.CpuType().empty() == true)
		{
			errc = SetOptions(R"({"cpu":"i8080"})");
			assert(!errc);
		}

		if(opt_.CpuType() == "i8080")
		{
			clock_ = MakeCpuClock(2000000);
			cpu_ = Make8080(systemBus_, std::bind(&Machine::ProcessControllers, this, std::placeholders::_1));
		}
	}

	std::error_code Machine::SetOptions(const char* options)
	{
		if (running_ == true)
		{
			return make_error_code(errc::busy);
		}

		return opt_.SetOptions(options);
	}

	ErrorCode Machine::SetClockResolution(int64_t clockResolution)
	{	
		if (running_ == true)
		{
			//return make_error_code(errc::busy);
			return ErrorCode::ClockResolution;
		}

		if (clockResolution < -1 || clockResolution > 10000000000)
		{
			//return make_error_code(errc::clock_resolution);
			return ErrorCode::ClockResolution;
		}

		if(clock_ == nullptr)
		{
			return ErrorCode::NoClock;
		}

		char str[32]{};
		snprintf(str, 32, "{\"clockResolution\":%" PRIi64 "}", clockResolution);
		opt_.SetOptions(str);
		//opt_.SetOptions(std::format(R"({{"clockResolution":{}}})", clockResolution).c_str());

		int64_t resInTicks = 0;

		auto errc = clock_->SetTickResolution(nanoseconds(clockResolution), &resInTicks);

		if (errc)
		{
			return ErrorCode::ClockResolution;
		}
		else
		{
			ticksPerIsr_ = opt_.ISRFreq() * resInTicks;
		}

		return ErrorCode::NoError;
	}

	void Machine::ProcessControllers(const SystemBus<uint16_t, uint8_t, 8>&& systemBus)
	{
		auto controlBus = systemBus.controlBus;
		auto addressBus = systemBus.addressBus;
		auto dataBus = systemBus.dataBus;

		if (memoryController_ != nullptr)
		{
			//check the control bus to see if there are any operations pending
			if (controlBus->Receive(Signal::MemoryRead))
			{
				dataBus->Send(memoryController_->Read(addressBus->Receive()));
			}

			if (controlBus->Receive(Signal::MemoryWrite))
			{
				memoryController_->Write(addressBus->Receive(), dataBus->Receive());
			}
		}

		if (ioController_ != nullptr)
		{
			if (controlBus->Receive(Signal::IoRead))
			{
				dataBus->Send(ioController_->Read(addressBus->Receive()));
			}

			if (controlBus->Receive(Signal::IoWrite))
			{
				ioController_->Write(addressBus->Receive(), dataBus->Receive());
			}
		}
	}

	// The probably needs to return std/tl::expected
	uint64_t Machine::Run(uint16_t pc)
	{
		if (memoryController_ == nullptr)
		{
			printf("Machine::Run: no memory controller has been set");
			return 0;
		}

		if (ioController_ == nullptr)
		{
			printf("Machine::Run: no io controller has been set");
			return 0;
		}

		if (running_ == true)
		{
			printf("Machine::Run: the machine is running");
			return 0;
		}

		if(clock_ == nullptr)
		{
			printf("Machine::Run: the clock is invalid\n");
			return 0;
		}

		if(cpu_ == nullptr)
		{
			printf("Machine::Run: the cpu is invalid\n");
			return 0;
		}

		cpu_->Reset(pc);
		clock_->Reset();
		SetClockResolution(opt_.ClockResolution());
		running_ = true;
		uint64_t totalTime = 0;
		auto launchPolicy = opt_.RunAsync() ? std::launch::async : std::launch::deferred;

		auto machineLoop = [this]
		{
			auto dataBus = systemBus_.dataBus;
			auto controlBus = systemBus_.controlBus;
			auto currTime = nanoseconds::zero();
			int64_t totalTicks = 0;
			int64_t lastTicks = 0;
#ifdef ENABLE_MEEN_SAVE
			auto loadLaunchPolicy = opt_.LoadAsync() ? std::launch::async : std::launch::deferred;
			auto saveLaunchPolicy = opt_.SaveAsync() ? std::launch::async : std::launch::deferred;
			std::future<std::string> onLoad;
			std::future<std::string> onSave;

			auto loadMachineState = [this](std::string&& str)
			{
				if (str.empty() == false)
				{
					// perform checks to make sure that this machine load state is compatible with this machine

					auto memUuid = memoryController_->Uuid();

					if (memUuid == std::array<uint8_t, 16>{})
					{
						return make_error_code(errc::incompatible_uuid);
					}
					
					auto json = nlohmann::json::parse(str, nullptr, false);

					if(json.is_discarded() == true)
					{
						return make_error_code(errc::json_parse);
					}
					
					if(!json.contains("memory"))
					{
						return make_error_code(errc::json_parse);
					}
					
					auto memory = json["memory"];

					if(!memory.contains("uuid") || !memory.contains("rom") || !memory.contains("ram"))
					{
						return make_error_code(errc::json_parse);
					}

					// The memory controllers must be the same
					auto jsonUuid = Utils::TxtToBin("base64", "none", 16, memory["uuid"].get<std::string>());

					if (jsonUuid.size() != memUuid.size() || std::equal(jsonUuid.begin(), jsonUuid.end(), memUuid.begin()) == false)
					{
						return make_error_code(errc::incompatible_uuid);
					}

					auto romMetadata = opt_.Rom();
					std::vector<uint8_t> rom;

					for (const auto& rm : romMetadata)
					{
						for (int addr = rm.first; addr < rm.first + rm.second; addr++)
						{
							rom.push_back(memoryController_->Read(addr));
						}
					}

					// The rom must be the same
					auto jsonMd5 = Utils::TxtToBin("base64", "none", 16, memory["rom"].get<std::string>());
					auto romMd5 = Utils::Md5(rom.data(), rom.size());

					if (jsonMd5.size() != romMd5.size() || std::equal(jsonMd5.begin(), jsonMd5.end(), romMd5.begin()) == false)
					{
						return make_error_code(errc::incompatible_rom);
					}

					// decode and decompress the ram
					auto jsonRam = memory["ram"];

					if(!jsonRam.contains("encoder") || !jsonRam.contains("compressor") || !jsonRam.contains("size") || !jsonRam.contains("bytes"))
					{
						return make_error_code(errc::json_parse);
					}

					if(jsonRam["encoder"].get<std::string>() != "base64")
					{
						return make_error_code(errc::json_config);
					}

					auto ram = Utils::TxtToBin(jsonRam["encoder"].get<std::string>(),
						jsonRam["compressor"].get<std::string>(),
						jsonRam["size"].get<uint32_t>(),
						jsonRam["bytes"].get<std::string>());
					
					auto ramMetadata = opt_.Ram();
					int ramSize = 0;
					int ramIndex = 0;

					for (const auto& rm : ramMetadata)
					{
						ramSize += rm.second;
					}

					// Make sure the ram size matches the layout
					if (ram.size() != ramSize)
					{
						return make_error_code(errc::incompatible_ram);
					}

					if(!json.contains("cpu"))
					{
						return make_error_code(errc::json_parse);
					}

					// Once all checks are complete, restore the cpu and the memory
					auto errc = cpu_->Load(json["cpu"].dump());

					if(errc)
					{
						return errc;
					}
					
					for (const auto& rm : ramMetadata)
					{
						for (int addr = rm.first; addr < rm.first + rm.second; addr++)
						{
							memoryController_->Write(addr, ram[ramIndex++]);
						}
					}
				}

				return make_error_code(errc::no_error);
			};

			auto checkHandler = [](std::future<std::string>& fut)
			{
				std::string str;

				if (fut.valid() == true)
				{
					auto status = fut.wait_for(nanoseconds::zero());

					if (status == std::future_status::deferred || status == std::future_status::ready)
					{
						str = fut.get();
					}
				}

				return str;
			};
#endif // ENABLE_MEEN_SAVE
			while (controlBus->Receive(Signal::PowerOff) == false)
			{
				//Execute the next instruction
				auto ticks = cpu_->Execute();
				currTime = clock_->Tick(ticks);
				totalTicks += ticks;

				// Check if it is time to service interrupts
				if (totalTicks - lastTicks >= ticksPerIsr_)
				{
					auto isr = ioController_->ServiceInterrupts(currTime.count(), totalTicks);

					switch (isr)
					{
						case ISR::Zero:
						case ISR::One:
						case ISR::Two:
						case ISR::Three:
						case ISR::Four:
						case ISR::Five:
						case ISR::Six:
						case ISR::Seven:
						{
							controlBus->Send(Signal::Interrupt);
							dataBus->Send(static_cast<uint8_t>(isr));
							break;
						}
						case ISR::Load:
						{
#ifdef ENABLE_MEEN_SAVE
							// If a user defined callback is set and we are not processing a load or save request
							if (onLoad_ != nullptr && onLoad.valid() == false && onSave.valid() == false)
							{
								onLoad = std::async(loadLaunchPolicy, [this]
								{
									std::string str;
										
									// TODO: this user defined method needs to be marked as nothrow
									auto json = onLoad_();

									if (json != nullptr)
									{
										// return a copy of the json c string as a std::string
										str = json;
									}
									else
									{
										printf("ISR::Load: the JSON string state to load is empty\n");
									}

									return str;
								});

								auto errc = loadMachineState(checkHandler(onLoad));

								if(errc)
								{
									printf("ISR::Load failed to load the machine state: %s\n", errc.message().c_str());
								}
							}
#endif // ENABLE_MEEN_SAVE
							break;
						}
						case ISR::Save:
						{
#ifdef ENABLE_MEEN_SAVE
							// If a user defined callback is set and we are not processing a save or load request
							if (onSave_ != nullptr && onSave.valid() == false && onLoad.valid() == false)
							{
								auto err = make_error_code(errc::no_error);
								auto memUuid = memoryController_->Uuid();

								if (opt_.Encoder() != "base64")
								{
									err = make_error_code(errc::json_config);
								}

								if (memUuid == std::array<uint8_t, 16>{})
								{
									err = make_error_code(errc::incompatible_uuid);
								}
								
								if(!err)
								{
									auto rm = [this](std::vector<std::pair<uint16_t, uint16_t>>&& metadata)
									{
										std::vector<uint8_t> mem;

										for (const auto& m : metadata)
										{
											for (auto addr = m.first; addr < m.first + m.second; addr++)
											{
												mem.push_back(memoryController_->Read(addr));
											}
										}

										return mem;
									};

									auto ram = rm(opt_.Ram());
									auto rom = rm(opt_.Rom());
									auto fmtStr = "{\"cpu\":%s,\"memory\":{\"uuid\":\"%s\",\"rom\":\"%s\",\"ram\":{\"encoder\":\"%s\",\"compressor\":\"%s\",\"size\":%d,\"bytes\":\"%s\"}}}";
									auto romMd5 = Utils::Md5(rom.data(), rom.size());

									// todo - replace snprintf with std::format
									auto writeState = [&](size_t& dataSize)
									{
										std::string str;
										char* data = nullptr;

										if (dataSize > 0)
										{
											str.resize(dataSize);
											data = str.data();
										}

										//cppcheck-suppress nullPointer
										dataSize = snprintf(data, dataSize, fmtStr,
											cpu_->Save().c_str(),
											Utils::BinToTxt("base64", "none", memUuid.data(), memUuid.size()).c_str(),
											Utils::BinToTxt("base64", "none", romMd5.data(), romMd5.size()).c_str(),
											opt_.Encoder().c_str(), opt_.Compressor().c_str(), ram.size(),
											Utils::BinToTxt(opt_.Encoder(), opt_.Compressor(), ram.data(), ram.size()).c_str()) + 1;

										return str;
									};

									size_t count = 0;
									writeState(count);

									onSave = std::async(saveLaunchPolicy, [this, state = writeState(count)]
									{
										// TODO: this method needs to be marked as nothrow
										onSave_(state.c_str());
										return std::string("");
									});

									checkHandler(onSave);
								}
								else
								{
									printf("ISR::Save failed: %s\n", err.message().c_str());
								}
							}
#endif // ENABLE_MEEN_SAVE
							break;
						}
						case ISR::Quit:
						{
#ifdef ENABLE_MEEN_SAVE
							// Wait for any outstanding load/save requests to complete

							if (onLoad.valid() == true)
							{
								// we are quitting, wait for the onLoad handler to complete
								auto errc = loadMachineState(onLoad.get());

								if(errc)
								{
									printf("ISR::Quit failed to load the machine state: %s\n", errc.message().c_str());
								}
							}

							if (onSave.valid() == true)
							{
								// we are quitting, wait for the onSave handler to complete
								onSave.get();
							}
#endif // ENABLE_MEEN_SAVE
							controlBus->Send(Signal::PowerOff);
							break;
						}
						case ISR::NoInterrupt:
						{
#ifdef ENABLE_MEEN_SAVE
							// no interrupts pending, do any work that is outstanding
							auto errc = loadMachineState(checkHandler(onLoad));
							
							if(errc)
							{
								printf("ISR::NoInterrupt failed to load the machine state: %s\n", errc.message().c_str());
							}
							
							checkHandler(onSave);
#endif // ENABLE_MEEN_SAVE
							break;
						}
						default:
						{
							//assert(0);
							break;
						}
					}

					lastTicks = totalTicks;
				}
			}

			return currTime.count();
		};

		fut_ = std::async(launchPolicy, [this, ml = std::move(machineLoop)]
		{
			return ml();
		});

		if (launchPolicy == std::launch::deferred)
		{
			totalTime = fut_.get();
			running_ = false;
		}

		return totalTime;
	}

	uint64_t Machine::WaitForCompletion()
	{
		uint64_t totalTime = 0;

		if (fut_.valid() == true)
		{
			totalTime = fut_.get();
			running_ = false;
		}

		return totalTime;
	}

	std::error_code Machine::SetMemoryController(const std::shared_ptr<IController>& controller)
	{
		if (controller == nullptr)
		{
			return make_error_code(errc::invalid_argument);
		}

		if (running_ == true)
		{
			return make_error_code(errc::busy);
		}

		memoryController_ = controller;
		return make_error_code(errc::no_error);
	}

	std::error_code Machine::SetIoController(const std::shared_ptr<IController>& controller)
	{
		if (controller == nullptr)
		{
			return make_error_code(errc::invalid_argument);
		}

		if (running_ == true)
		{
			return make_error_code(errc::busy);
		}

		ioController_ = controller;
		return make_error_code(errc::no_error);
	}

	std::error_code Machine::OnSave(std::function<void(const char* json)>&& onSave)
	{
#ifdef ENABLE_MEEN_SAVE
		if (running_ == true)
		{
			return make_error_code(errc::busy);
		}

		onSave_ = std::move(onSave);
		return make_error_code(errc::no_error);
#else
		return make_error_code(errc::not_implemented);
#endif // ENABLE_MEEN_SAVE
	}

	std::error_code Machine::OnLoad(std::function<const char*()>&& onLoad)
	{
#ifdef ENABLE_MEEN_SAVE
		if (running_ == true)
		{
			return make_error_code(errc::busy);
		}

		onLoad_ = std::move(onLoad);
		return make_error_code(errc::no_error);
#else
		return make_error_code(errc::not_implemented);
#endif // ENABLE_MEEN_SAVE
	}

	std::string Machine::Save() const
	{
#ifdef ENABLE_MEEN_SAVE
		if (running_ == true)
		{
			return "Machine::Save: the machine is running, save failed";
		}

		if (memoryController_ == nullptr)
		{
			return "Machine::Save: no memory controller set, save failed";
		}

		auto rm = [this](std::vector<std::pair<uint16_t, uint16_t>>&& metadata)
		{
			std::vector<uint8_t> mem;

			for (const auto& m : metadata)
			{
				for (auto addr = m.first; addr < m.first + m.second; addr++)
				{
					mem.push_back(memoryController_->Read(addr));
				}
			}

			return mem;
		};

		auto rom = rm(opt_.Rom());
		auto ram = rm(opt_.Ram());
		auto fmtStr = "{\"cpu\":%s,\"memory\":{\"uuid\":\"%s\",\"rom\":\"%s\",\"ram\":{\"encoder\":\"%s\",\"compressor\":\"%s\",\"size\":%d,\"bytes\":\"%s\"}}}";
		auto memUuid = memoryController_->Uuid();
		auto romMd5 = Utils::Md5(rom.data(), rom.size());
		auto writeState = [&](char* data, size_t dataSize)
		{
			auto count = snprintf(data, dataSize, fmtStr,
				cpu_->Save().c_str(),
				Utils::BinToTxt("base64", "none", memUuid.data(), memUuid.size()).c_str(),
				Utils::BinToTxt("base64", "none", romMd5.data(), romMd5.size()).c_str(),
				opt_.Encoder().c_str(), opt_.Compressor().c_str(), ram.size(),
				Utils::BinToTxt(opt_.Encoder(), opt_.Compressor(), ram.data(), ram.size()).c_str());
			return count;
		};

		auto count = writeState(nullptr, 0) + 1;
		std::string state(count, '\0');
		writeState(state.data(), count);
		return state;
#else
		return "Machine::Save: save support disabled, save failed";
#endif // ENABLE_MEEN_SAVE
	}

	std::unique_ptr<uint8_t[]> Machine::GetState(int* size) const
	{
		if (running_ == true)
		{
			return nullptr;
		}

		if(cpu_ == nullptr)
		{
			return nullptr;
		}

		return cpu_->GetState(size);
	}
} // namespace MachEmu