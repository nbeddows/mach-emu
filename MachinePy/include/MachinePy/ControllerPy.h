#ifndef CONTROLLERPY_H
#define CONTROLLERPY_H

#include "Controller/IController.h"

namespace MachEmu
{
    struct ControllerPy final : public MachEmu::IController
    {
        uint8_t Read(uint16_t address) final;
        void Write(uint16_t address, uint8_t value) final;
        MachEmu::ISR ServiceInterrupts(uint64_t currTime, uint64_t cycles) final;
        std::array<uint8_t, 16> Uuid() const final;
    };
} // namespace MachEmu

#endif // CONTROLLERPY_H