#include <pybind11/pybind11.h>

#include "MachinePy/MachineHolder.h"

namespace MachEmu
{
	MachineHolder::MachineHolder()
	{
		machine_ = MachEmu::MakeMachine();
	}

	MachineHolder::MachineHolder(const char* json)
	{
		machine_ = MachEmu::MakeMachine(json);
	}

	ErrorCode MachineHolder::SetClockResolution(int64_t clockResolution)
	{
		return machine_->SetClockResolution(clockResolution);
	}

	void MachineHolder::OnSave(std::function<void(std::string&& json)>& onSave)
	{
		return machine_->OnSave(std::move(onSave));
	}

	std::string MachineHolder::Save() const
	{
		return machine_->Save();
	}

	uint64_t MachineHolder::Run(uint16_t offset)
	{
		return machine_->Run(offset);
	}

	void MachineHolder::SetIoController(MachEmu::IController* controller)
	{
		// custom deleter, don't delete this pointer from c++, python owns it
		machine_->SetIoController(std::shared_ptr<MachEmu::IController>(controller, [](MachEmu::IController*) {}));
	}

	void MachineHolder::SetMemoryController(MachEmu::IController* controller)
	{
		machine_->SetMemoryController(std::shared_ptr<MachEmu::IController>(controller, [](MachEmu::IController*) {}));
	}

	ErrorCode MachineHolder::SetOptions(const char* options)
	{
		return machine_->SetOptions(options);
	}

	uint64_t MachineHolder::WaitForCompletion()
	{
		// WaitForCompletion is a long running function that does not interract with Python.
		// Release the Python Global Interpreter Lock so the calling script doesn't stall.
		pybind11::gil_scoped_release nogil{};
		return machine_->WaitForCompletion();
	}
} // namespace MachEmu
