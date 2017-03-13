from ctypes import *
libTest = cdll.LoadLibrary('./UdacitySim/TestClientNode/libTestClientNode.so')
class TestClientNode(object):
    def __init__(self, sensorfunc):
        self.obj = libTest.TestClientNode_new()
        self.sensorfunc = sensorfunc

        # set sensor data available callback
        self.FUNC1 = CFUNCTYPE(
            None,                              # returns nothing, self
            c_double, c_double, c_double,      # steering, throttle and braking
            c_double, c_double,                # lat, lon
            c_double, c_double,                # yaw, heading
            c_double, c_double, c_double,      # vel (x, y, z)
            c_int, POINTER(c_ubyte))           # encoded size and JPEG encoded image
        self.func1 = self.FUNC1(self.sensorfunc)
        libTest.TestClientNode_setSensorCallback(self.obj, self.func1)

    def connectPolySync(self):
        libTest.TestClientNode_connectPolySync(self.obj)

    def sendCommand(self, steering, throttle, brake):
        libTest.TestClientNode_sendCommand(
            self.obj, c_double(steering), c_double(throttle), c_double(brake))

libSim = cdll.LoadLibrary('./UdacitySim/SimNode/libSimNode.so')
class SimNode(object):
    def __init__(self, requestsensorfunc, simcommandfunc):
        self.obj = libSim.SimNode_new()
        self.requestsensorfunc = requestsensorfunc
        self.simcommandfunc = simcommandfunc

        # set sensor data available callback
        self.FUNC1 = CFUNCTYPE( None )         # returns nothing
        self.func1 = self.FUNC1(self.requestsensorfunc)
        libSim.SimNode_setRequestSensorCallback(self.obj, self.func1)

        # set command data available callback
        self.FUNC2 = CFUNCTYPE(
            None,                              # returns nothing
            c_double, c_double, c_double)      # steering, throttle and braking commands
        self.func2 = self.FUNC2(self.simcommandfunc)
        libSim.SimNode_setCommandCallback(self.obj, self.func2)

    def connectPolySync(self):
        libSim.SimNode_connectPolySync(self.obj)

    def setSimulatorValues(self, steering, throttle, brake, lat, lon, orient1, orient2, orient3, orient4, heading, velx, vely, velz):
        libSim.SimNode_sendSensorValues(
            self.obj,
            c_double(steering), c_double(throttle), c_double(brake), c_double(lat), c_double(lon),
            c_double(orient1), c_double(orient2), c_double(orient3), c_double(orient4),
            c_double(velx), c_double(vely), c_double(velz))

    def setSimulatorImage(self, width, height, depth, size, buf):
        libSim.SimNode_sendSensorImage(
            self.obj, c_int(width), c_int(height), c_int(depth), c_uint(size), c_char_p(buf))

