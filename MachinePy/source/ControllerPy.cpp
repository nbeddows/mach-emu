#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "MachinePy/ControllerPy.h"

namespace MachEmu
{
    uint8_t ControllerPy::Read(uint16_t address)
    {
        PYBIND11_OVERRIDE_PURE(
            uint8_t,            /* Return type */
            IController,        /* Parent class */
            Read,               /* Name of function in C++ (must match Python name) */
            address             /* Argument(s) */
        );
    };

    void ControllerPy::Write(uint16_t address, uint8_t value)
    {
        PYBIND11_OVERRIDE_PURE(
            void,               /* Return type */
            IController,        /* Parent class */
            Write,              /* Name of function in C++ (must match Python name) */
            address,            /* Argument(s) */
            value
        );
    };

    MachEmu::ISR ControllerPy::ServiceInterrupts(uint64_t currTime, uint64_t cycles)
    {
        PYBIND11_OVERRIDE_PURE(
            MachEmu::ISR,       /* Return type */
            IController,        /* Parent class */
            ServiceInterrupts,  /* Name of function in C++ (must match Python name) */
            currTime,           /* Argument(s) */
            cycles
        );
    }

    std::array<uint8_t, 16> ControllerPy::Uuid() const
    {
        using Uint8Array16 = std::array<uint8_t, 16>;

        PYBIND11_OVERRIDE_PURE(
            Uint8Array16,       /* Return type */
            IController,        /* Parent class */
            Uuid                /* Name of function in C++ (must match Python name) */
        );
    }
}