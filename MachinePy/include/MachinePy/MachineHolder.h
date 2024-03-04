#ifndef MACHINEHOLDER_H
#define MACHINEHOLDER_H

#include "Machine/MachineFactory.h"

namespace MachEmu
{
    // Factory and Machine wrapper
    class MachineHolder final
    {
    private:
        std::unique_ptr<MachEmu::IMachine> machine_;
    public:
        MachineHolder();

        uint64_t Run(uint16_t offset);
        
        ErrorCode SetClockResolution(int64_t clockResolution);
        void SetIoController(MachEmu::IController* controller);
        void SetMemoryController(MachEmu::IController* controller);
    };
} // namespace MachEmu

#endif // MACHINEHOLDER_H