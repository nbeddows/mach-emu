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

#ifndef ICONTROLLER_H
#define ICONTROLLER_H

import <cstdint>;
#include "Base/Base.h"

namespace MachEmu
{
	/** Device interface

		An interface to a device that can interact with the cpu.

		@todo	Can be made into a template which can accept different
				address and data sizes, currently only support 8 bit
				data read and write from 16 bit addresses.
	*/
	struct IController
	{
		/** Read from a device
		
			Reads 8 bits from a device at the specifed 16 bit address.

			The implementation of the function should be lightweight and
			should never block. Blocking on this function will cause
			the cpu pipeline to stall, hence slowing down the emulated
			application.

			@param	address		The 16 bit address to read from.

			@return	uint8_t		The 8 bits of data read from the device.
		*/
		virtual uint8_t Read(uint16_t address) = 0;

		/** Write to device
		
			Write 8 bits of data to a device at the specified 16 bit address.

			The implementation of the function should be lightweight and
			should never block. Blocking on this function will cause
			the cpu pipeline to stall, hence slowing down the emulated
			application.

			@param	address		The 16 bit address to write to.
			@param	value		The 8 bit data value to write.
		*/
		virtual void Write(uint16_t address, uint8_t value) = 0;

		/** Interrupt handler
		
			Query the device for any pending interrupts.

			@param	currTime	The time in nanoseconds of the machine clock.
			@param	cycles		The total number of cpu cycles that have elapsed.

			@return	ISR			The interrupt service routine generated by the
								device.

			@remark				ISR::Quit exits the main control loop when returned from
								an io controller interrupt handler.

			@see				ISR
		*/
		virtual ISR ServiceInterrupts(uint64_t currTime, uint64_t cycles) = 0;

		/** Destroys the controller
		
			Release all resources used by this controller instance.
		*/
		virtual ~IController() = default;
	};
} // namespace MachEmu

#endif // ICONTROLLER_H
