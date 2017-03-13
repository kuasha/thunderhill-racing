import argparse
import base64
import json
import cv2

import numpy as np
import socketio
import eventlet
import eventlet.wsgi
import time
from PIL import Image
from PIL import ImageOps
from flask import Flask, render_template
from io import BytesIO

from geopy.distance import vincenty
from mpl_toolkits.basemap import Basemap

import queue
from threading import Thread

from UdacitySim.PublishSubscribe import SimNode

start = (39.53745, -122.33879)
mt = Basemap(llcrnrlon=-122.341041,llcrnrlat=39.532678,urcrnrlon=-122.337929,urcrnrlat=39.541455,
    projection='merc',lon_0=start[1],lat_0=start[0],resolution='h')

ch, img_rows, img_cols = 3, 160, 320
current_steering_angle = 0.0
current_throttle = 0.0
initial = True

diffx = -989.70
diffy = -58.984

def toGPS(simx, simy):
    projx, projy = simx+diffx, simy + diffy
    lon, lat = mt(projx, projy,inverse=True)
    return [lon, lat]

sio = socketio.Server()
app = Flask(__name__)

inq = queue.Queue()
outq = queue.Queue()

@sio.on('telemetry')
def telemetry(sid, data):
    # The current position of the car in the simulator
    position = [float(n) for n in data['position'].split(':')]

    # set the GPS coordinates from the simulator position.
    data['GPS'] = toGPS(position[0], position[1])

    # send it to the queue
    inq.put(data)

    # let everyone know we queued.
    print("queue.Queue.put!  ", inq.qsize())

    # delay in sending back a timely response (50 milliseconds or 20 times/second)
    time.sleep(0.05)

    # send back the current command to keep the connection active
    send_control(current_steering_angle, current_throttle)


@sio.on('connect')
def connect(sid, environ):
    print("connect ", sid)
    send_control(current_steering_angle, current_throttle)


def send_control(steering_angle, throttle):
    sio.emit("steer", data={
    'steering_angle': steering_angle.__str__(),
    'throttle': throttle.__str__()
    }, skip_sid=True)

# diagnostics routines
def hexdump(byte_array):
    for i in range(10):
        print(''.join('{:02x}'.format(byte_array[i*16+j]) for j in range(16)))

# diagnostics routines
def saveJpegImage(image):
    f = open('driveNode_out.jpeg', 'wb')
    f.write(image)
    f.close()

# class to keep polysync connection issolated
class SimClient():
    def requestSensorData(self):
        global initial
        global img_cols
        global img_rows
        global ch
        if not inq.empty():
            z = inq.qsize()
            print("inq.qsize(): ", z)
            for i in range(z):
                x = inq.get()

            # set steering angle
            steering = float(x['steering_angle'])

            # throttle and brake
            if float(x['throttle']) > 0:
                throttle = float(x['throttle'])
                braking = 0.0
            else:
                throttle = 0.0
                braking = -float(x['throttle'])

            # lat lon
            lat = float(x['GPS'][0])
            lon = float(x['GPS'][1])

            print("steering, throttle, braking:", steering, throttle, braking)
            print("lat, lon:", lat, lon)
            print("requestSensorData called")

            # send the sensor info to polysync
            self.node.setSimulatorValues( steering, throttle, braking, lat, lon, -1.0, 2.0, 1.0, -13.0, 0.0, -1.0, 1.1, 0.2)

            # process image
            imgString = x['image']
            self.jpegImage = base64.b64decode(imgString)

            # uncomment to set up image diagnostics
            # hexdump(self.jpegImage)
            # saveJpegImage(self.jpegImage)
            size = len(self.jpegImage)

            # calculate the initial image to get its real dimensions
            if initial:
                image = Image.open(BytesIO(self.jpegImage))
                print("image:", type(image))
                image_array = np.asarray(image)
                print("image_array:", type(image_array), image_array.shape)
                transformed_image = image_array[:, :, :]
                img_cols, img_rows, ch = transformed_image.shape
                initial = False

            # send the image info to polysync
            self.node.setSimulatorImage( img_cols, img_rows, ch, size, self.jpegImage)

    def receiveCommands(self, steering, throttle, braking ):
        global current_steering_angle
        global current_throttle
        print("received steering, throttle, braking:", steering, throttle, braking)
        current_steering_angle = steering
        current_throttle = throttle - braking

    def __init__(self):
        self.node = SimNode(self.requestSensorData, self.receiveCommands)
        self.node.connectPolySync()
                

def startSimClient():
    client = SimClient()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Remote Driving')
    args = parser.parse_args()

    # wrap Flask application with engineio's middleware
    app = socketio.Middleware(sio, app)

    # start wsgi server thread
    thread = Thread(target = eventlet.wsgi.server, args=(eventlet.listen(('', 4567)), app))
    # thread = Thread(target = startSimClient)
    thread.start()

    # eventlet.wsgi.server(eventlet.listen(('', 4567)), app)
    startSimClient()

    # wait for wsgi or simclient thread to end
    thread.join()

