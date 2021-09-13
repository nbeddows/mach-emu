export module IMachine;

import <memory>;
import IController;

namespace Emulator
{
	/** IMachine

	*/
	export struct IMachine
	{
		/** Run
		
			Run the roms loaded into memory initialising execution at the given
			program counter.

			@param	pc		The program counter.

			@discussion		The program counter is the memory address at which the cpu
							will start executing the instructions contained in the
							rom files that were loaded into memory.
		*/
		virtual void Run(uint16_t pc) = 0;

		/** SetMemoryController
		
			Set a custom memory controller with the machine.

			@param	controller	The memory controller to be used with this machine.
		*/
		virtual void SetMemoryController (std::shared_ptr<IController> controller) = 0;

		/** SetIoController

			Set a custom io controller with the machine.

			@param	controller	The io controller to be used with this machine.
		*/
		virtual void SetIoController (std::shared_ptr<IController> controller) = 0;

		virtual ~IMachine() = default;
	};
}