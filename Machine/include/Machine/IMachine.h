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

#ifndef IMACHINE_H
#define IMACHINE_H

#include <functional>
#include <memory>
#include <string>
#include "Controller/IController.h"

namespace MachEmu
{
	/** Machine interface

		An abstract representation of a basic machine with a cpu, clock and
		custom memory and IO.

		Basic Principles of Operation:

		@code{.cpp}

		// Create a synchronous i8080 machine running as fast as possible 
		auto machine = MakeMachine();

		// Create a custom memory controller (See tests for examples)
		auto customMemoryController = std::make_unique<CustomMemoryController>();

		// Load memory with program via custom controller method
		customMemoryController->LoadProgram("myProgram.com");

		// Create custom IO Controller (See tests for examples)
		auto customIOController = std::make_unique<CustomIOController>();

		// Set the memory and IO controllers with the machine
		machine->SetIOController(customIOController);
		machine->SetMemoryController(customMemoryController);

		// set the clock resolution - not setting this will run the
		// machine as fast as possible (default)
		machine->SetOptions(R"({"clockResolution":20000000})"); // 20 milliseconds (50Hz)

		// Run the machine - this is a blocking function, it won't return until the custom IO
		// controller ServiceInterrupts override generates an ISR::Quit interrupt.
		auto runTime = machine->Run();

		// Run the machine asychronously - this can be done by setting the following json config either
		// in the MakeMachine factory method or via IMachine::SetOptions: `{"runAsync":true}`
		machine->SetOptions(R"({"runAsync":true})");
		machine->Run();
		runTime = machine->WaitForCompletion();

		@endcode
	*/
	struct IMachine
	{
		/** Run the machine

			Run the roms loaded into memory initialising execution at the given
			program counter.

			@param	pc					The program counter is the memory address at
										which the cpu will start executing the instructions
										contained in the rom files that were loaded into memory.

			@return						The duration of the run time of the machine as a uint64_t in nanoseconds.

			@throws						std::runtime_error if no memory or io controller has been set on this
										machine.

			@remark						When no program counter is specified cpu instruction
										execution will begin from memory address 0x00.

			@remark						When the run asynchronous configuration option is set to true
										the return value will always be 0.

			@see SetMemoryController
		*/
		virtual uint64_t Run(uint16_t pc = 0x00) = 0;

		/** Wait for the machine to finish running

			Block the current thread until the machine execution loop has completed.

			@return						The duration of the run time of the machine as a uint64_t in nanoseconds.

			@remark						When the run asynchronous option is set to false (default) or the Run method has not been called
										this method will return 0 immediatley.
		*/
		virtual uint64_t WaitForCompletion() = 0;

		/** Set a custom memory controller

			The machine will use this controller when it needs to read
			and/or write data to ram.

			@param	controller		The memory controller to be used with this machine.

			@throws					std::invalid_argument exception when controller is nullptr.

			@throws					std::runtime_error if the machine is currently running.

			@remark					See Tests/TestControllers/MemoryController.cpp
		*/
		virtual void SetMemoryController (const std::shared_ptr<IController>& controller) = 0;

		/** Set a custom io controller

			The machine will use this controller when it needs to read
			and/or write to an io device.

			@param	controller	The io controller to be used with this machine.

			@throws				std::invalid_argument when the controller is nullptr.

			@throws				std::runtime_error when the machine is currently running.

			@remark				See Tests/TestControllers/TestIoController.cpp.
		*/
		virtual void SetIoController (const std::shared_ptr<IController>& controller) = 0;

		/** Set machine options

			@param		options		A json string specifying the desired options to update. Passing in an options string of nullptr will set all options
									to their defaults.

			@return					ErrorCode::NoError: all options were set successfully.<br>
									ErrorCode::UnknownOption: all recognised options were set successfully though unrecognised options were found.

			@throws					std::runtime_error or any other exception that the underlying json parser throws (nlohmann json)

			@throws					std::runtime_error when the cpu option is specified or if the machine is currently running.

			@throws					std::invalid_argument when any option value is illegal.

			@remark					The `cpu` configuration option can only be set via the factory machine constructor.

			@see					MakeMachine for supported configuration options.
		*/
		virtual ErrorCode SetOptions(const char* options) = 0;

		/** Save the state of the machine.
		
			Upon saving the state of the machine, call the method `onSave` to process the save state.
		
			The json layout of an example save state is specifed in the json snippet below:

			@code{.json}

			{
				"cpu":
				{
					"uuid":"O+hPH516S3ClRdnzSRL8rQ==",	// The base64 unique identifier of the cpu
					"registers":
					{
						"a":0,							// The a register
						"b":0,							// The b register
						"c":0,							// The c register
						"d":0,							// The d register
						"e":0,							// The e register
						"h":0,							// The h register
						"l":0,							// The l register
						"s":2							// The status register
					},
					"pc":0,								// The program counter
					"sp":0								// The stack pointer
				},
			{
			"memory":
			{
				"uuid":"zRjYZ92/TaqtWroc666wMQ==",		// The base64 unique identifier for the memory controller
				"rom":"FkgjfhUYrudiMj7y65789Io=",		// The base64 MD5 hash of the rom
				"ram":
				{
					"encoder":"base64",					// The binary to text encoding for the ram
					"compressor":"zlib",				// The compressor to use for the ram
					"size":57343,						// The size of the uncompressed ram
					"bytes":"... the ram bytes ..."		// The compressed and encoded ram bytes
				}
			}

			@endcode

			@param	onSave		The method to call once the save state has been processed.

			@since version 1.5.0
		*/
		virtual void OnSave(std::function<void(std::string&& json)>&& onSave) = 0;

		/** Save the state of the machine.

			Returns the state of the machine as a JSON string.

			@return				A copy of the internal state of the machine as a JSON string.

			@throws				std::runtime_error if the machine is currently running.

			@remark				The returned state of the machine currently just contains the cpu

			<table>
			<tr><td>Name</td><td>Value</td></tr>
			<tr><td>name</td><td>Cpu type as a sting, ie; "i8080"</td></tr>
			<tr><td>a</td><td>Contents of the a register</td></tr>
			<tr><td>b</td><td>Contents of the b register</td></tr>
			<tr><td>c</td><td>Contents of the c register</td></tr>
			<tr><td>d</td><td>Contents of the d register</td></tr>
			<tr><td>e</td><td>Contents of the e register</td></tr>
			<tr><td>h</td><td>Contents of the h register</td></tr>
			<tr><td>l</td><td>Contents of the l register</td></tr>
			<tr><td>s</td><td>Contents of the status register</td></tr>
			<tr><td>pc</td><td>Contents of the program counter</td></tr>
			<tr><td>sp</td><td>Contents of the stack pointer</td></tr>
			</table>
		*/
		virtual std::string Save() const = 0;

		/**	Set the frequency at which the internal clock ticks.

			@param		clockResolution				A request in nanoseconds as to how frequently the
													machine clock will tick. The clock is disabled by
													default (run as fast as possible).

			@return									ErrorCode::NoError: The resolution was set successfully.<br>
													ErrorCode::ClockResolution: The resolution was set,
													however, the host does not support a high enough resolution
													timer for this resolution. This may result in high CPU usage,
													high jitter and inaccurate timing.
													This method should be called again with a lower resolution.

			@throws									std::runtime_error if the machine is currently running.

			@remark		Note that this is only a request and while best efforts are made to honour it, the consistency of the tick
						rate will not be perfect, especially at higher resolutions when no high resolution clock is available.

						A value of less than 0 will run the machine as fast as possible with the highest possible resolution.
						A value of 0 will run the machine at realtime (or as close to) with the highest possible resolution.
						Note that a value of between 0 and a millisecond (1000000 nanoseconds) will always spin the cpu to maintain
						the clock speed and is not recommended.

			@deprecated	since 1.4.0

			@see		SetOptions
		*/
		[[deprecated("Will be removed in v2.0.0, please use SetOptions")]] virtual ErrorCode SetClockResolution (int64_t clockResolution) = 0;

		/**	Get the state of the machine.

			@param		size	Storage for the state array size in bytes. Optional value and is set to nullptr (ignore) by default.
								The size of the state array can be obtained via the table below.

			@return				A copy of the internal state of the machine as a uint8_t unique_ptr array.

			@throws				std::runtime_error if the machine is currently running.

			@remark				The returned state of the machine currently just contains the cpu as an array of bytes
								in the following form:

			<table>
			<tr><td>Cpu</td><td>Registers</td><td>Status</td><td>Program Counter</td><td>Stack Pointer</td><td>Total Bits</td></tr>
			<tr><td>Intel8080</td><td>A B C D E H L (8 bits each)</td><td>S (8 bits)</td><td>PC (16 bits)</td><td>SP (16 bits)</td><td>96</td></tr>
			</table>

			@deprecated			since 1.4.0
		*/
		[[deprecated("Will be removed in v2.0.0, please use std::string GetState()")]] virtual std::unique_ptr<uint8_t[]> GetState(int* size = nullptr) const = 0;

		/** Destruct the machine

			Release all resources used by this machine instance.
		*/
		virtual ~IMachine() = default;
	};
} // namespace MachEmu

#endif // IMACHINE_H