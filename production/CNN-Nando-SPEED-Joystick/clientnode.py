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
from keras.models import model_from_json, load_model 
from keras.preprocessing.image import ImageDataGenerator, array_to_img, img_to_array
from keras.optimizers import Adam
from keras import backend as K
#from Preprocess import *
import cv2
import time
from data_buffer import DataBuffer
import queue
import threading
import json


f = h5py.File("model.h5", mode='r')
model_version = f.attrs.get('keras_version')
keras_version = str(keras_version).encode('utf8')

if model_version != keras_version:
	print('You are using Keras version ', keras_version, ', but the model was built using ', model_version)


def customLoss(y_true, y_pred):
	return K.mean(K.square(y_pred - y_true), axis=-1)

model = load_model("model.h5", custom_objects={'customLoss':customLoss})
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

X = []
Y = []

def make_prediction():
	global graph
	print('make prediction')

	### recording flag is replaced with training flag to start image data collection
	training = True

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
					transformed_image_array = image_array[None, :, :, :]
					prediction = model.predict(transformed_image_array, batch_size=1)[0]
					steering_angle = float(prediction[0])
					throttle = 0.0 #float(prediction[1])
					brake = 0.0 #float(prediction[2])

					if brake > 0.5:
						throttle = -brake

					if res_queue.full(): # maintain a single most recent prediction in the queu
						res_queue.get(False)
					# save only steering predictions
					res_queue.put((steering_angle, throttle, brake))

					# fill the training and testing queue
					if len(X) < 100:
						X.append(transformed_image_array)
						Y.append(steering_angle)
					else:
						if len(X) == 100:
							print("Ready for training!")

					if training:
						#print("Training: ")
						#print("Right Stick Left|Right Axis value {:>6.3f}".format(leftright) )
						#print("Right Stick Up|Down Axis value {:>6.3f}".format(updown) )
						X.append(transformed_image_array)
						Y.append(steering_angle)

def sendValues():
	steer = 0.0
	throttle = 0.0
	brake = 0.0
	while 1:
		try:
			prediction = res_queue.get(block=False)
			print(prediction)
			steer = float(prediction[0])
			throttle = float(prediction[1])
			brake = float(prediction[2])
			print("got values: ", steer, throttle, brake)
		except queue.Empty:
			pass
		Node.steerCommand(steer)
		# use cruise control
		Node.throttleCommand(throttle)
		Node.brakeCommand(brake)
		time.sleep(0.01)

def batchgen(X, Y):
	while 1:
		i = int(random()*len(X))
		y = Y[i]
		image = X[i]
		y = np.array([[y]])
		image = image.reshape(1, img_cols, img_rows, ch)
		yield image, y

def model_trainer(fileModelJSON):
	print("Model Trainer Thread Starting...")

	fileWeights = fileModelJSON.replace('json', 'h5')
	with open(fileModelJSON, 'r') as jfile:
		model = model_from_json(json.load(jfile))

	adam = Adam(lr=0.00001, beta_1=0.9, beta_2=0.999, epsilon=1e-08, decay=0.0)
	model.compile(optimizer=adam, loss="mse", metrics=['accuracy'])
	model.load_weights(fileWeights)
	print("Loaded model from disk:")
	model.summary()

	# start training loop...
	while 1:
		if len(X) > 100:
			batch_size = 20
			samples_per_epoch = int(len(X)/batch_size)
			val_size = int(samples_per_epoch/10)
			if val_size < 10:
				val_size = 10
			nb_epoch = 100

			history = model.fit_generator(batchgen(X,Y),
					samples_per_epoch=samples_per_epoch, nb_epoch=nb_epoch,
					validation_data=batchgen(X,Y),
					nb_val_samples=val_size,
					verbose=1)

			print("Saving model to disk: ",fileModelJSON,"and",fileWeights)
			if Path(fileModelJSON).is_file():
				os.remove(fileModelJSON)
			json_string = model.to_json()
			with open(fileModelJSON,'w' ) as f:
				json.dump(json_string, f)
			if Path(fileWeights).is_file():
				os.remove(fileWeights)
			model.save_weights(fileWeights)
		else:
			print("Not Ready!  Sleeping for 5...")
			sleep(5)


if __name__ == '__main__':
	#parser = argparse.ArgumentParser(description='Remote Driving')
	#parser.add_argument('model', type=str,
	#	help='Path to model definition json. Model weights should be on the same path.')
	#args = parser.parse_args()

	thread = threading.Thread(target=make_prediction, args=())
	thread.daemon = True
	thread.start()

	thread2 = threading.Thread(target=sendValues, args=())
	thread2.daemon = True
	thread2.start()

	# start training thread
	thread3 = threading.Thread(target = model_trainer, args=('modelTrained.json'))
	thread3.daemon = True
	thread3.start()

	Node.connectPolySync()
