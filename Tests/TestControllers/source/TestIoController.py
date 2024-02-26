from BaseIoController import BaseIoController
from MachEmuPy import ISR

class TestIoController(BaseIoController):
    def __init__(self):
        super().__init__()
        self.__deviceData = 0xAA
        self.__lastTime = 0

    def Read(self, deviceNumber):
        if deviceNumber == 0:
            return self.__deviceData
        else:
            return 0x00

    def Write(self, deviceNumber, value):
        if deviceNumber == 0:
            self.__deviceData = value
        else:
            super().Write(deviceNumber, value)

    def ServiceInterrupts(self, currTime, cycles):
        isr = super().ServiceInterrupts(currTime, cycles)

        if isr == ISR.NoInterrupt:
            t = currTime - self.__lastTime

            if t >= 0:
                if t > 1000000000:
                    isr = ISR.One
            else:
                self.__lastTime = 0

        return isr
