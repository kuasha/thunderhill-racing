import argparse
import base64
import json

import array
import numpy as np
import socketio
import eventlet
import eventlet.wsgi
import time
from PIL import Image
from PIL import ImageOps
from flask import Flask, render_template
from io import BytesIO

from keras.models import model_from_json
from keras.preprocessing.image import ImageDataGenerator, array_to_img, img_to_array

sio = socketio.Server()
app = Flask(__name__)
model = None
prev_image_array = None

import ctypes

from UdacitySim.PublishSubscribe import TestClientNode

def copyImage(byte_array, imageSize):
    if imageSize > 8:
        ctypes.resize(byte_array, imageSize)
        image = []
        for i in range(imageSize):
            image.append(byte_array[i])
        return array.array('B', image).tostring()
    return byte_array

def hexdump(byte_array):
    for i in range(10):
        print(''.join('{:02x}'.format(byte_array[i*16+j]) for j in range(16)))

def saveJpegImage(image):
    f = open('testclient_out.jpeg', 'wb')
    f.write(image)
    f.close()

class TestClient():
    # haveSensorData will only be invoked after an image message is sent.
    def haveSensorData(
            self, steering, throttle, braking, lat, lon,
            yaw, heading, velX, velY, velZ, imageSize, rawImage):

        print("steering, throttle, braking:", steering, throttle, braking)
        print("lat, lon:", lat, lon)
        print("yaw:", yaw)
        print("heading:", heading)
        print("velocity (x, y, z):", velX, velY, velZ)


        self.jpegImage = copyImage(rawImage, imageSize)
        print("jpegImage:", type(self.jpegImage), imageSize)

        # uncomment to enable image diagnostics
        # hexdump(self.jpegImage)
        # saveJpegImage(self.jpegImage)

        image = Image.open(BytesIO(self.jpegImage))
        print("image:", type(image))

        image_array = np.asarray(image)
        print("image_array:", type(image_array), image_array.shape)
        transformed_image_array = image_array[None, :, :, :]

        #################################################################################
        # TODO: 1. This should be replaced by the model prediction
        #       2. May need to expose message timestamps to discard old sensor readings.
        #################################################################################
        # delay a bit (50 milliseconds)
        #################################################################################
        time.sleep(0.05)
        steering_angle = -0.2
        throttle = 0.5
        braking = 0.0
        #################################################################################
        # This model currently assumes that the features of the model are just the images.
        # Feel free to change this.
        # TODO: Uncomment below to predict using your model...
        #################################################################################
        #
        # steering_angle = float(model.predict(transformed_image_array, batch_size=1))
        #
        #################################################################################
        # send steering and throttle commands back through polysync
        self.client.sendCommand( steering_angle, throttle, braking)

    def __init__(self):
        self.client = TestClientNode(self.haveSensorData)
        self.client.connectPolySync()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Polysync Remote Driving')
    #
    # TODO: uncomment below to load model
    #
    # parser.add_argument('model', type=str,
    # help='Path to model definition json. Model weights should be on the same path.')
    # args = parser.parse_args()
    # with open(args.model, 'r') as jfile:
    #     model = model_from_json(json.load(jfile))
    # model.compile("adam", "mse")
    # weights_file = args.model.replace('json', 'h5')
    # model.load_weights(weights_file)
    testclient = TestClient()
