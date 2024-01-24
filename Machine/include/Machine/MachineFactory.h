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

#ifndef MACHINE_FACTORY_H
#define MACHINE_FACTORY_H

import <memory>;
#include "IMachine.h"

#ifdef _WINDOWS
#ifdef MachEmu_EXPORTS
#define DLL_EXP_IMP __declspec(dllexport)
#else
#define DLL_EXP_IMP __declspec(dllimport)
#endif
#else
#ifdef MachEmu_EXPORTS
#define DLL_EXP_IMP [[gnu::visibility("default")]]
#else
#define DLL_EXP_IMP
#endif
#endif

namespace MachEmu
{
	/** Create a machine.
	
		Build a machine based on the Intel 8080 cpu.

		@return		std::unique_ptr<IMachine>	An empty i8080 machine that can be loaded with memory and io controllers.
	*/
	DLL_EXP_IMP std::unique_ptr<IMachine> Make8080Machine();
} // namespace MachEmu

#endif // MACHINE_FACTORY_H