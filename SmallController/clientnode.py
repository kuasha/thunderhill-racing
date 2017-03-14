
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

f = h5py.File("ps_small_model.h5", mode='r')
model_version = f.attrs.get('keras_version')
keras_version = str(keras_version).encode('utf8')

if model_version != keras_version:
    print('You are using Keras version ', keras_version, ', but the model was built using ', model_version)


model = load_model("ps_small_model.h5")
graph = tf.get_default_graph()


def copyImage(byte_array, imageSize):
    if imageSize > 8:
        resize(byte_array, imageSize)
        image = []
        for i in range(imageSize):
            image.append(byte_array[i])
        return array.array('B', image).tostring()
    return byte_array


def imageReceived(imageSize, rawImage):
	global graph
	with graph.as_default():
		jpegImage = copyImage(rawImage, imageSize)
		image = Image.open(BytesIO(jpegImage))
		image_array = np.asarray(image)
		steering_angle = float(model.predict(image_array[None, :, :, :], batch_size=1))
		print(steering_angle)
		Node.steer(c_float(steering_angle))


Node = MainNode(imageReceived)
Node.connectPolySync()