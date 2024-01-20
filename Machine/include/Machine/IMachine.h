/*
Copyright (c) 2021-2023 Nicolas Beddows <nicolas.beddows@gmail.com>

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

import <memory>;
#include "Controller/IController.h"

namespace MachEmu
{
	/** Machine interface

		An abstract representation of a basic machine with a cpu, clock and
		custom memory and IO.

		Basic Principles of Operation:

		@code{.cpp}

		// Create a machine
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
		machine->SetResolution(20000000); // 20 millisecond clock resolution (50Hz)

		// Run the machine
		auto runTime = machine->Run();

		// machine->Run() is a blocking function, it won't return until the custom IO
		// controller ServiceInterrupts override generates an ISR::Quit interrupt.

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
										When no program counter
										is specified cpu instruction execution will
										begin from memory address 0x00.

			@return						The duration of the run time of the machine as a uint64_t in nanoseconds.

			@throw	std::runtime_error	No memory or io controller has been set on this
										machine.

			@see SetMemoryController
		*/
		virtual uint64_t Run(uint16_t pc = 0x00) = 0;

		/** Set a custom memory controller

			The machine will use this controller when it needs to read
			and/or write data to ram.
			
			@param	controller		The memory controller to be used with this machine.

			@throw					An invalid_argument exception when controller is nullptr.

			@remark					See Tests/TestControllers/MemoryController.cpp
		*/
		virtual void SetMemoryController (const std::shared_ptr<IController>& controller) = 0;

		/** Set a custom io controller

			The machine will use this controller when it needs to read
			and/or write to an io device.

			@param	controller	The io controller to be used with this machine.

			@throw				An invalid_argument exception when controller is nullptr.

			@remark				See Tests/TestControllers/TestIoController.cpp.
		*/
		virtual void SetIoController (const std::shared_ptr<IController>& controller) = 0;

		/**	Set the frequency at which the internal clock ticks.

			@param		clockResolution				A request in nanoseconds as to how frequently the
													machine clock will tick. The clock is disabled by
													default (run as fast as possible).

			@return									ErrorCode::NoError: The resolution was set successfully.
													ErrorCode::ClockResolution: The resolution was set,
													however, the host does not support a high enough resolution
													timer for this resolution. This may result in high CPU usage,
													high jitter and inaccurate timing.
													This method should be called again with a lower resolution.

			@remark		Note that this is only a request and while best efforts are made to honour it, the consistency of the tick
						rate will not be perfect, especially at higher resolutions when no high resolution clock is available.
		
						A value of less than 0 will run the machine as fast as possible with the highest possible resolution.
						A value of 0 will run the machine at realtime (or as close to) with the highest possible resolution.
						Note that a value of between 0 and a millisecond (1000000 nanoseconds) will always spin the cpu to maintain
						the clock speed and is not recommended.
		*/
		virtual ErrorCode SetClockResolution (int64_t clockResolution) = 0;

		/**	Get the state of the machine.

			Returns the state of the machine (currently just the cpu) as an array in the following
			form dependent on cpu type:

			<table>
			<tr><td>Cpu</td><td>Registers</td><td>Status</td><td>Program Counter</td><td>Stack Pointer</td></tr>
			<tr><td>Intel8080</td><td>A B C D E H L (8 bits each)</td><td>S (8 bits)</td><td>PC (16 bits)</td><td>SP (16 bits)</td></tr>
			</table>

			Remaining bits are unused.
		*/
		virtual std::array<uint8_t, 12> GetState() const = 0;

		/** Destruct the machine

			Release all resources used by this machine instance.
		*/
		virtual ~IMachine() = default;
	};
} // namespace MachEmu

#endif // IMACHINE_H