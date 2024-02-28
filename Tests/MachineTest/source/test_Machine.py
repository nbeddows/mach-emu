import sys
import re
import unittest

from MachEmuPy import __version__
from MachEmuPy import ErrorCode
from MachEmuPy import Make8080Machine

# relative path to Python controller test modules
sys.path.append('../../TestControllers/source')
# relative path to the C++ controller test module
sys.path.append('../../../bin/')

# import Python controller modules (a port of the c++ modules below)
# always use the c++ memory controller module for performance reasons, the python module is available strictly for demonstration purposes
#from MemoryController import MemoryController
from TestIoController import TestIoController
from CpmIoController import CpmIoController

# These c++ controller module versions are also available (CpmIoController currently not working)
# always use the c++ memory controller for performance reasons
from TestControllersPy import MemoryController
#from TestControllersPy import CpmIoController
#from TestControllersPy import TestIoController

class MachineTest(unittest.TestCase):
    def setUp(self):
        self.programsDir = '../../Programs/'
        self.memoryController = MemoryController()
        self.cpmIoController = CpmIoController(self.memoryController)
        self.testIoController = TestIoController()
        self.machine = Make8080Machine()
        err = self.machine.SetClockResolution(-1)
        self.assertEqual(err, ErrorCode.NoError)
        self.machine.SetIoController(self.testIoController)
        self.machine.SetMemoryController(self.memoryController)

    def test_Version(self):
        self.assertTrue(re.match(r'^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$', __version__))

    def test_SetNoIoController(self):
        with self.assertRaises(ValueError):
            self.machine.SetIoController(None)

    def test_SetNoMemoryController(self):
        with self.assertRaises(ValueError):
            self.machine.SetMemoryController(None)

    def test_BadClockResolution(self):
        err = self.machine.SetClockResolution(0)
        self.assertEqual(err, ErrorCode.ClockResolution)

    def test_RunTimed(self):
        self.memoryController.Load(self.programsDir + 'nopStart.bin', 0x0000)
        self.memoryController.Load(self.programsDir + 'nopEnd.bin', 0xC34F)

        err = self.machine.SetClockResolution(16666667) # 60Hz
        self.assertEqual(err, ErrorCode.NoError)

        nanos = 0
        iterations = 5

        for i in range(iterations):
            nanos += self.machine.Run(0)

        error = nanos / 5 - 1000000000
        self.assertTrue(error >= 0 and error <= 500000)

    def test_8080Pre(self):
        self.memoryController.Load(self.programsDir + 'exitTest.bin', 0x0000)
        self.memoryController.Load(self.programsDir + 'bdosMsg.bin', 0x0005)
        self.memoryController.Load(self.programsDir + '8080PRE.COM', 0x0100)
        self.machine.SetIoController(self.cpmIoController)
        self.machine.Run(0x0100)
        self.assertIn('8080 Preliminary tests complete', self.cpmIoController.Message())

    def test_Tst8080(self):
        self.memoryController.Load(self.programsDir + 'exitTest.bin', 0x0000)
        self.memoryController.Load(self.programsDir + 'bdosMsg.bin', 0x0005)
        self.memoryController.Load(self.programsDir + 'TST8080.COM', 0x0100)
        self.machine.SetIoController(self.cpmIoController)
        self.machine.Run(0x0100)
        self.assertIn('CPU IS OPERATIONAL', self.cpmIoController.Message())

    # this will take a little while to complete
    def test_Cpu8080(self):
        self.memoryController.Load(self.programsDir + 'exitTest.bin', 0x0000)
        self.memoryController.Load(self.programsDir + 'bdosMsg.bin', 0x0005)
        self.memoryController.Load(self.programsDir + 'CPUTEST.COM', 0x0100)
        self.machine.SetIoController(self.cpmIoController)
        self.machine.Run(0x0100)
        self.assertIn('CPU TESTS OK', self.cpmIoController.Message())

    # this will take a long time to complete
    def test_8080Exm(self):
        self.memoryController.Load(self.programsDir + 'exitTest.bin', 0x0000)
        self.memoryController.Load(self.programsDir + 'bdosMsg.bin', 0x0005)
        self.memoryController.Load(self.programsDir + '8080EXM.COM', 0x0100)
        self.machine.SetIoController(self.cpmIoController)
        self.machine.Run(0x0100)
        self.assertNotIn('ERROR', self.cpmIoController.Message())

if __name__ == '__main__':
    unittest.main()