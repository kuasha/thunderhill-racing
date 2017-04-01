from MainNode.MainNode import MainNode
from ctypes import *
import array
from PIL import Image
from io import BytesIO
import numpy as np
from keras.models import load_model
import h5py
from keras import __version__ as keras_version
import tensorflow as tf
from keras import backend as K
import cv2
import time
from data_buffer import DataBuffer
import queue
import threading
import argparse
import base64
import json

import numpy as np
import time
from PIL import Image
from PIL import ImageOps

from keras.models import model_from_json
from keras.preprocessing.image import ImageDataGenerator, array_to_img, img_to_array



f = h5py.File("thunderhill_slow.h5", mode='r')
model_version = f.attrs.get('keras_version')
keras_version = str(keras_version).encode('utf8')

if model_version != keras_version:
	print('You are using Keras version ', keras_version, ', but the model was built using ', model_version)

model = load_model("thunderhill_slow.h5")

graph = tf.get_default_graph()

data_buffer = DataBuffer()
res_queue = queue.Queue(maxsize=1)


def copyImage(byte_array, imageSize):
    new_array = np.ctypeslib.as_array(byte_array,shape=(imageSize,)).reshape((960, 480, 3))
    return new_array


def imageReceived(imageSize, rawImage, speed, lat, lon):
    jpegImage = copyImage(rawImage, imageSize)
    data_buffer.add_item((jpegImage, speed, lat, lon))
    try:
        prediction = res_queue.get(block=False)
        Node.steerCommand(c_float(prediction[0]))
        Node.throttleCommand(c_float(prediction[1]))
        Node.brakeCommand(c_float(prediction[2]))
    except queue.Empty:
        pass

Node = MainNode(imageReceived)

def make_prediction():
    global graph
    # print('make prediction')
    while True:
        with graph.as_default():
            item = data_buffer.get_item_for_processing()
            if item and len(item) == 4:
                import time
                start = time.time()
                jpeg_image = item[0]
                speed = item[1]
                lat = item[2]
                lon = item[3]
                
                img = np.array(Image.frombytes('RGB', [960,480], jpeg_image, 'raw'))[::-1,:,:]

                output = model.predict([img[None, :, :, :], np.array([speed])[None,:]])
                steering_angle = output[0][0]
                throttle = output[0][1]

                min_speed = 8
                max_speed = 30

                if float(speed) > max_speed:
                    throttle = 0

                if steering_angle > 0.2*6.28 or steering_angle < -0.2*6.28:
                    throttle = -throttle


                brake=0
                if throttle<0:
                    throttle=0
                    brake=np.abs(throttle)

                print('prediction:',steering_angle,throttle,brake, 'speed: ',speed)

                if res_queue.full(): # maintain a single most recent prediction in the queue
                    res_queue.get(False)

                res_queue.put((steering_angle, throttle, brake))
                end = time.time()
                print('\n%.3f ms\n'%(end - start))

def sendValues():
    steer = 0
    throttle = 0
    brake = 0
    while 1:
        try:
            prediction = res_queue.get(block=False)
            steer = c_float(prediction[0])
            throttle = c_float(prediction[1])
            brake = c_float(prediction[2])
            print("got values: ", steer, throttle, brake)
        except queue.Empty:
            pass
        Node.steerCommand(steer)
        Node.throttleCommand(throttle)
        Node.brakeCommand(brake)
        time.sleep(0.01)

thread = threading.Thread(target=make_prediction, args=())
thread.daemon = True
thread.start()
thread2 = threading.Thread(target=sendValues, args=())
thread2.daemon = True
thread2.start()


Node.connectPolySync()
