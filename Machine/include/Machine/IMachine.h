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
	/** Machine interface.

	*/
	struct IMachine
	{
		/** Run the machine.
		
			Run the roms loaded into memory initialising execution at the given
			program counter.

			@param	pc					The program counter is the memory address at
										which the cpu will start executing the instructions
										contained in the rom files that were loaded into memory.
										When no program counter
										is specified cpu instruction execution will
										begin from memory address 0x00.

			@throw	std::runtime_error	No memory controller has been set on this
										machine.

										@see SetMemoryController
		*/
		virtual void Run(uint16_t pc = 0x00) = 0;

		/** Set a custom memory controller with the machine.

			Not setting a memory controller will cause the Run function
			to throw a std::runtime_error exception.

			@param	controller	The memory controller to be used with this machine.
		*/
		virtual void SetMemoryController (const std::shared_ptr<IController>& controller) = 0;

		/** Set a custom io controller with the machine.

			@param	controller	The io controller to be used with this machine.

			Not setting an io controller, while valid, is NOT recommended.
			This is because without an io controller the machine will have
			no means of exiting via an i/o device or via an i/o device
			interrupt.

			A valid reason for having a null io controller would be if you
			wanted to run a machine for a certain period of time.

			For example;

			auto machine = MakeMachine();
			auto memoryController = MakeDefaultMemoryController(16);
			memoryController.Load ("myProgram.bin");
			machine.SetMemoryController (memoryController_);
			//Don't set an io controller

			auto future = std::async(std::launch::async, [&]
			{
				machine.Run();
			});

			auto status = future.wait_for(seconds(2));

			//One can check that status of the machine and
			//memory after 2 seconds of run time.
		*/
		virtual void SetIoController (const std::shared_ptr<IController>& controller) = 0;

		/** Destructor.

			Release all resources used by this machine instance.
		*/
		virtual ~IMachine() = default;
	};
} // namespace MachEmu

#endif // IMACHINE_H