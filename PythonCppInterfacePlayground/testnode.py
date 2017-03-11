from PolysyncNode.Recorder import RecorderNode

def haveLatLon(lat, lon):
    print("lat, lon:", lat, lon)

def haveYawHeading(yaw, heading):
    print("yaw:", yaw)
    print("heading:", heading)

def haveVelocity(velocityX, velocityY, velocityZ):
    print("velocity(x,y,z):", velocityX, velocityY, velocityZ)

def haveImage(image):
    print("image:", type(image), len(image))

Recorder = RecorderNode(haveLatLon, haveYawHeading, haveVelocity, haveImage)
Recorder.connectPolySync()

