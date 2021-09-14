export module IController;

import <chrono>;
import <cstdint>;
import <filesystem>;
import Base;

using namespace std::chrono;

namespace Emulator
{
	/** IController

		A interface to a device that can interact with the cpu.

		@todo	Can be made into a template which can accept different
				address and data sizes.
	*/
	export struct IController
	{
		/** Read
		
			Read 8 bits from a device at the specifed 16 bit address.

			@param	address		The 16 bit address to read from.

			@return	uint8_t		The 8 bits of data read from the device.
		*/
		virtual uint8_t Read(uint16_t address) = 0;

		/** Write
		
			Write 8 bits of data to a device at the specified 16 bit address.

			@param	address		The 16 bit address to write to.
			@param	value		The 8 bit data value to write.
		*/
		virtual void Write(uint16_t address, uint8_t value) = 0;

		/** Service Interrupts
		
			Query the device for any pending interrupts.

			@param	currTime	The time in nanoseconds of the cpu clock.

			@return	ISR			The interrupt service routine generated by the
								device.

			@discussion			A return value of 0 indicates no interrupts
								have been triggered.
		*/
		virtual ISR ServiceInterrupts(nanoseconds currTime) = 0;

		virtual ~IController() = default;
	};

	/** IMemoryController
	
		An interface to a generic memory device.
	*/
	export struct IMemoryController : public IController
	{
		/** Load
		
			@param	romFile				The binary file containing cpu instructions to be
										loaded into memory.
			@param	address				The memory address at which the romFile is to be
										loaded at.

			@throw	runtime_error		The rom file failed to open, for example, the path does
										not exist.
			@throw	length_error		The rom file size is too big to fit at the specified
										memory offet.
			@throw	invalid_argument	The rom file exists, but it failed to load.
			
			@todo	add other load variant interfaces, like from vector, array, etc
		*/
		virtual void Load(std::filesystem::path romFile, uint16_t offset) = 0;
		
		/**	Size
		
			Obtains the size of the allocated memory used by this memory device.

			@return	size_t		The size of memory allocated by this device.
		*/
		virtual size_t Size() const = 0;

		virtual ~IMemoryController() = default;
	};
}
