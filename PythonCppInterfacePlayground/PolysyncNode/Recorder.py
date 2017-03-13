from ctypes import *
lib = cdll.LoadLibrary('./PolysyncNode/libRecorderNode.so')

class RecorderNode(object):
    def __init__(self, latlonfunc, yawfunc, orientfunc, imagefunc):
        self.obj = lib.RecorderNode_new()
        self.latlonfunc = latlonfunc
        self.yawfunc = yawfunc
        self.orientfunc = orientfunc
        self.imagefunc = imagefunc

        # set lat/long callback
        self.FUNC1 = CFUNCTYPE(None, c_double, c_double)
        self.func1 = self.FUNC1(self.latlonfunc)
        lib.RecorderNode_setLatLonCallback(self.obj, self.func1)

        # set yawheading callback
        self.FUNC2 = CFUNCTYPE(None, c_double, c_double)
        self.func2 = self.FUNC2(self.yawfunc)
        lib.RecorderNode_setYawCallback(self.obj, self.func2)

        # set velocity callback
        self.FUNC3 = CFUNCTYPE(None, c_double, c_double, c_double)
        self.func3 = self.FUNC3(self.orientfunc)
        lib.RecorderNode_setVelocityCallback(self.obj, self.func3)

        # set image callback
        self.FUNC4 = CFUNCTYPE(None, POINTER(c_ubyte))
        self.func4 = self.FUNC4(self.imagefunc)
        lib.RecorderNode_setImageCallback(self.obj, self.func4)

    def connectPolySync(self):
        lib.RecorderNode_connectPolySync(self.obj)

