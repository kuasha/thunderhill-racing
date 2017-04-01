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
from Preprocess import *
import cv2
import time
from data_buffer import DataBuffer
import queue
import threading


f = h5py.File("psyncModel.h5", mode='r')
model_version = f.attrs.get('keras_version')
keras_version = str(keras_version).encode('utf8')

if model_version != keras_version:
	print('You are using Keras version ', keras_version, ', but the model was built using ', model_version)


def customLoss(y_true, y_pred):
	return K.mean(K.square(y_pred - y_true), axis=-1)

model = load_model("psyncModel.h5", custom_objects={'customLoss':customLoss})
graph = tf.get_default_graph()

data_buffer = DataBuffer()
res_queue = queue.Queue(maxsize=1)

idxs = [0, 1, 2]
means = [-2.135234308696, 0.690051203865, 62.68238949]
stds = [0.000022089013, 0.000045442627, 13.48539298]

debug = False


def normalize_vector(xVec):
	for i, mean, std in zip(idxs, means, stds):
		xVec[i] -= mean
		xVec[i] /= std
	return xVec


def copyImage(byte_array, imageSize):
	if imageSize > 8:
		resize(byte_array, imageSize)
		image = []
		for i in range(imageSize):
			image.append(byte_array[i])
		return array.array('B', image).tostring()
	return byte_array


def imageReceived(imageSize, rawImage, speed, lat, lon):
	print("image received with: ", speed, lat, lon)
	jpegImage = copyImage(rawImage, imageSize)
	data_buffer.add_item((jpegImage, speed, lat, lon))
	
Node = MainNode(imageReceived)

def make_prediction():
	global graph
	print('make prediction')
	while True:
		with graph.as_default():
			item = data_buffer.get_item_for_processing()
			if item and len(item) == 4:
				jpeg_image = item[0]
				speed = item[1]
				lat = item[2]
				lon = item[3]

				if jpeg_image:
					image = np.array(Image.frombytes('RGB', [960,480], jpeg_image, 'raw'))
					image_array = np.asarray(image)
					image_array = cv2.cvtColor(image_array, cv2.COLOR_BGR2RGB)
					image_array = cv2.resize(image_array, (160, 80))
					prediction = model.predict(image_array[None, :, :, :], batch_size=1)[0]
					steering_angle = float(prediction[0])
					throttle = float(prediction[1])
					brake = float(prediction[2])

					if brake > 0.5:
						throttle = -brake

					# print('prediction: ',steering_angle, throttle, brake, speed)

					if res_queue.full(): # maintain a single most recent prediction in the queue
						res_queue.get(False)
					res_queue.put((steering_angle, throttle, brake))


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
