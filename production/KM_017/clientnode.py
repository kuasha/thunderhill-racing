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
# from Preprocess import *
import cv2
import time
from data_buffer import DataBuffer
import queue
import threading
# import argparse
# import base64
import json
from glob import glob
import numpy as np
# import socketio
# import eventlet
# import eventlet.wsgi
# import time
from PIL import Image
from PIL import ImageOps
# from flask import Flask, render_template
from io import BytesIO

from keras.models import model_from_json
from keras.preprocessing.image import ImageDataGenerator, array_to_img, img_to_array

# Fix error with Keras and TensorFlow
import tensorflow as tf
import cv2

from transformations import *

def customLoss(y_true, y_pred):
    return K.mean(K.square(y_pred - y_true), axis=-1)

with open('model.json', 'r') as jfile:
    model = model_from_json(jfile.read())

model.compile("adam", ["mse",'categorical_crossentropy'])

weights_list=sorted(glob("weights.*"))

weights_file = weights_list[-1]
print(weights_file)
model.load_weights(weights_file)

graph = tf.get_default_graph()

data_buffer = DataBuffer()
res_queue = queue.Queue(maxsize=1)


def copyImage(byte_array, imageSize):
    new_array = np.ctypeslib.as_array(byte_array,shape=(imageSize,)).reshape((960, 480, 3))
    return new_array



def imageReceived(imageSize, rawImage, speed, lat, lon):
    jpegImage = copyImage(rawImage, imageSize)
    data_buffer.add_item((jpegImage, speed, lat, lon))

Node = MainNode(imageReceived)

def make_prediction():
    global graph
    while True:
        with graph.as_default():
            item = data_buffer.get_item_for_processing()
            if item and len(item) == 4:
                jpeg_image = item[0]
                speed = item[1]
                lat = item[2]
                lon = item[3]
                img = np.array(Image.frombytes('RGB', [960,480], jpeg_image, 'raw'))

                image_array = cv2.resize(img, (320, 160))
                image_array = cv2.cvtColor(image_array, cv2.COLOR_BGR2RGB)
                image_array = Preproc(image_array)

                image_array=image_array.reshape(1,image_array.shape[0],image_array.shape[1],image_array.shape[2])

                res=model.predict([image_array], batch_size=1)
                if res_queue.full(): # maintain a single most recent prediction in the queue
                    res_queue.get(False)
                steering_angle = res[0]

                print('speed: %8.3f'%speed,'predictions: %12s'%(str(steering_angle)))

                res_queue.put((steering_angle, 0,0))


def sendValues():
    steer = 0
    throttle = 0
    brake = 0
    while 1:
        try:
            prediction = res_queue.get(block=False)
            steer = c_float(prediction[0])
            # throttle = c_float(prediction[1])
            # brake = c_float(prediction[2])
        except queue.Empty:
            pass
        Node.steerCommand(-steer)
        # Node.throttleCommand(throttle)
        # Node.brakeCommand(brake)
        time.sleep(0.01)

thread = threading.Thread(target=make_prediction, args=())
thread.daemon = True
thread.start()
thread2 = threading.Thread(target=sendValues, args=())
thread2.daemon = True
thread2.start()

Node.connectPolySync()
