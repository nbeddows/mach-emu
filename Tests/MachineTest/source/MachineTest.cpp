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

import <memory>;
import MemoryController;
import TestIoController;

#include <gtest/gtest.h>
// Needs to be declared after gtest due to g++/gtest
// compilation issues: fixme
import CpmIoController;
#include "Controller/IController.h"
#include "Machine/IMachine.h"
#include "Machine/MachineFactory.h"
#include "nlohmann/json.hpp"

namespace MachEmu::Tests
{
	class MachineTest : public testing::Test
	{
	protected:
		static std::shared_ptr<IController> cpmIoController_;
		static std::shared_ptr<MemoryController> memoryController_;
		static std::shared_ptr<IController> testIoController_;
		static std::unique_ptr<IMachine> machine_;

		static void LoadAndRun(const char* name, const char* expected);
		static void Run(bool runAsync);
		static void Load(bool runAsync);
	public:
		static void SetUpTestCase();
		void SetUp();
	};

	std::shared_ptr<IController> MachineTest::cpmIoController_;
	std::shared_ptr<MemoryController> MachineTest::memoryController_;
	std::shared_ptr<IController> MachineTest::testIoController_;
	std::unique_ptr<IMachine> MachineTest::machine_;

	void MachineTest::SetUpTestCase()
	{
		// Note that the tests don't require a json string to be set as it just uses the default values,
		// it is used here for demonstation purposes only
		machine_ = MakeMachine(R"({"cpu":"i8080"})");
		memoryController_ = std::make_shared<MemoryController>();
		cpmIoController_ = std::make_shared<CpmIoController>(static_pointer_cast<IController>(memoryController_));
		testIoController_ = std::make_shared<TestIoController>();
	}

	void MachineTest::SetUp()
	{
		memoryController_->Clear();
		//CP/M Warm Boot is at memory address 0x00, this will be
		//emulated with the exitTest subroutine.
		memoryController_->Load(PROGRAMS_DIR"/exitTest.bin", 0x00);
		//CP/M BDOS print message system call is at memory address 0x05,
		//this will be emulated with the bdosMsg subroutine.
		memoryController_->Load(PROGRAMS_DIR"/bdosMsg.bin", 0x05);
		machine_->SetMemoryController(memoryController_);
		machine_->SetIoController(testIoController_);
		auto err = machine_->SetOptions(R"({"clockResolution":-1,"isrFreq":0,"runAsync":false})");
		EXPECT_EQ(ErrorCode::NoError, err);
	}

	void MachineTest::LoadAndRun(const char* name, const char* expected)
	{
		EXPECT_NO_THROW
		(
			machine_->OnSave([expected](std::string&& actual)
			{
				auto actualJson = nlohmann::json::parse(actual);
				auto expectedJson = nlohmann::json::parse(expected);
				EXPECT_STREQ(expectedJson.dump().c_str(), actualJson["cpu"].dump().c_str());
			});

			std::string dir = PROGRAMS_DIR"/";
			memoryController_->Load((dir + name).c_str(), 0x100);
			machine_->Run(0x100);
		);
	}

	TEST_F(MachineTest, SetNullptrMemoryController)
	{
		EXPECT_ANY_THROW
		(
			//cppcheck-suppress unknownMacro
			machine_->SetMemoryController(nullptr);
		);
	}

	TEST_F(MachineTest, SetNullptrIoController)
	{
		EXPECT_ANY_THROW
		(
			//cppcheck-suppress unknownMacro
			machine_->SetIoController(nullptr);
		);
	}

	TEST_F(MachineTest, SetCpuAfterConstruction)
	{
		EXPECT_ANY_THROW
		(
			//cppcheck-suppress unknownMacro
			machine_->SetOptions(R"({"cpu":"i8080"})");
		);
	}

	TEST_F(MachineTest, NegativeISRFrequency)
	{
		EXPECT_ANY_THROW
		(
			//cppcheck-suppress unknownMacro
			machine_->SetOptions(R"({"isrFreq":-1.0})");
		);
	}

	TEST_F(MachineTest, MethodsThrowAfterRunCalled)
	{
		//cppcheck-suppress unknownMacro
		// Set the resolution so the Run method takes about 1 second to complete therefore allowing subsequent IMachine method calls to throw
		auto err = machine_->SetOptions(R"({"clockResolution":25000000,"runAsync":true})"); // must be async so the Run method returns immediately

		// This is currently not supported on some platforms
		if (err == ErrorCode::NotImplemented)
		{
			return;
		}

		memoryController_->Load(PROGRAMS_DIR"nopStart.bin", 0x04);
		memoryController_->Load(PROGRAMS_DIR"nopEnd.bin", 0xC353);

		EXPECT_NO_THROW
		(
			machine_->Run(0x04);
		);

		EXPECT_ANY_THROW
		(
			machine_->Run(0x100);
		);

		EXPECT_ANY_THROW
		(
			machine_->SetOptions(R"({"isrFreq":1})");
		);

		EXPECT_ANY_THROW
		(
			machine_->SetMemoryController(memoryController_);
		);

		EXPECT_ANY_THROW
		(
			machine_->SetIoController(testIoController_);
		);

		EXPECT_ANY_THROW
		(
			machine_->OnLoad([]() {return "";});
		);

		EXPECT_ANY_THROW
		(
			machine_->OnSave([](std::string&&){});
		);

		// Since we are running async we need to wait for completion
		machine_->WaitForCompletion();

		// We are now no longer running, all these methods shouldn't throw

		EXPECT_NO_THROW
		(
			machine_->SetOptions(R"({"isrFreq":1})");
		);

		EXPECT_NO_THROW
		(
			machine_->SetMemoryController(memoryController_);
		);

		EXPECT_NO_THROW
		(
			machine_->SetIoController(testIoController_);
		);

		EXPECT_NO_THROW
		(
			machine_->OnLoad([]() {return "";});
		);

		EXPECT_NO_THROW
		(
			machine_->OnSave([](std::string&&) {});
		);
	}

	void MachineTest::Run(bool runAsync)
	{
		EXPECT_NO_THROW
		(
			ErrorCode err;

			if (runAsync == true)
			{
				err = machine_->SetOptions(R"({"runAsync":true})");

				// This is currently not supported on some platforms
				if (err == ErrorCode::NotImplemented)
				{
					return;
				}
			}

			// Run a program that should take a second to complete
			// (in actual fact it's 2000047 ticks, 47 ticks over a second.
			// We need to be as close a possible to 2000000 ticks without
			// going under so the cpu sleeps at the end
			// of the program so it maintains sync. It's never going to
			// be perfect, but its close enough for testing purposes).
			memoryController_->Load(PROGRAMS_DIR"nopStart.bin", 0x04);
			memoryController_->Load(PROGRAMS_DIR"nopEnd.bin", 0xC353);

			// 25 millisecond resolution
			err = machine_->SetOptions(R"({"clockResolution":25000000})");
			EXPECT_EQ(ErrorCode::NoError, err);

			int64_t nanos = 0;

			// If an over sleep occurs after the last batch of instructions are executed during a machine run
			// there is no way to compensate for this which means running a timed test just once will result in
			// sporadic failures. To counter this we will run the machine multiples times and take the average
			// of the accumulated run time, this should smooth out the errors caused by end of program over sleeps.
			int64_t iterations = 1;

			for (int i = 0; i < iterations; i++)
			{
				if (runAsync == true)
				{
					machine_->Run(0x04);
					nanos += machine_->WaitForCompletion();
				}
				else
				{
					nanos += machine_->Run(0x04);
				}
			}

			auto error = (nanos / iterations) - 1000000000;
			// Allow an average 500 micros of over sleep error
			EXPECT_EQ(true, error >= 0 && error <= 500000);
		);
	}

	TEST_F(MachineTest, RunTimed)
	{
		Run(false);
	}

	TEST_F(MachineTest, RunTimedAsync)
	{
		Run(true);
	}

	void MachineTest::Load(bool runAsync)
	{
		EXPECT_NO_THROW
		(
			ErrorCode err;

			if (runAsync == true)
			{
				err = machine_->SetOptions(R"({"runAsync":true})");

				// This is currently not supported on some platforms
				if (err == ErrorCode::NotImplemented)
				{
					return;
				}
			}

			std::vector<std::string> saveStates;
			// Call the out instruction
			memoryController_->Write(0x0098, 0xD3);
			// The data to write to the controller that will trigger the ISR::Load interrupt 
			memoryController_->Write(0x0099, 0xFD);
			memoryController_->Load(PROGRAMS_DIR"/TST8080.COM", 0x100);
			
			err = machine_->SetOptions(R"({"romOffset":0,"romSize":8192,"ramOffset":8192,"ramSize":57343})");
			EXPECT_EQ(ErrorCode::NoError, err);
			machine_->SetIoController(cpmIoController_);
			machine_->OnSave([&](std::string&& json)
			{
				saveStates.emplace_back(json);
			});
			machine_->OnLoad([&]()
			{
				// 0 - mid program save state, 1 and 2 - end of program save states
				return saveStates[0];
			});

			machine_->Run(0x0100);
			
			if (runAsync == true)
			{
				machine_->WaitForCompletion();
			}

			// run it again, but this time trigger the load interrupt
			machine_->Run(0x0098);

			if (runAsync == true)
			{
				machine_->WaitForCompletion();
			}

			ASSERT_EQ(saveStates.size(), 3);
			EXPECT_STREQ(R"({"cpu":{"uuid":"O+hPH516S3ClRdnzSRL8rQ==","registers":{"a":19,"b":19,"c":0,"d":19,"e":0,"h":19,"l":0,"s":86},"pc":1236,"sp":1981},"memory":{"uuid":"zRjYZ92/TaqtWroc666wMQ==","rom":"gi09EdL3h5T/Q+SpMgKhdg==","ram":{"encoder":"base64","compressor":"zlib","size":57343,"bytes":"eJztwTEBAAAAwqD1T20IX6AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA4Dbf/wAB"}}})", saveStates[0].c_str());
			EXPECT_STREQ(saveStates[1].c_str(), saveStates[2].c_str());
		);
	}

	TEST_F(MachineTest, OnLoad)
	{
		Load(true);
	}

	TEST_F(MachineTest, OnLoadAsync)
	{
		Load(false);
	}

	#include "8080Test.cpp"
} // namespace MachEmu::Tests
