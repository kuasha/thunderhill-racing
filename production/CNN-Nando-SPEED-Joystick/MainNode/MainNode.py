from ctypes import *
import pygame

lib = cdll.LoadLibrary('./MainNode/libMainNode.so')

class MainNode(object):
	def __init__(self, imagefunc):
		self.obj = lib.MainNode_new()
		self.imagefunc = imagefunc

		# set img callback
		self.FUNC1 = CFUNCTYPE(None, c_int, POINTER(c_ubyte), c_float, c_float, c_float)
		self.func1 = self.FUNC1(self.imagefunc)
		lib.MainNode_setImageCallback(self.obj, self.func1)

		# initialize pygame and joystick
		pygame.init()
		pygame.joystick.init()


	def steerCommand(self, angle):
		# We are using PS3 left joystick: so axis (0,1) run in pairs, left/right for 2, up/down for 3
		# Change this if you want to switch to another axis on your joystick!
		# Normally they are centered on (0,0)
		leftright = joystick.get_axis(0)
		lib.MainNode_steerCommand(self.obj, angle+leftright)

	def brakeCommand(self, value):
		# We are using PS3 left joystick: so axis (0,1) run in pairs, left/right for 2, up/down for 3
		# Change this if you want to switch to another axis on your joystick!
		# Normally they are centered on (0,0)
		updown = joystick.get_axis(1)
		if updown < 0.0:
			updown = -updown
		else:
			updown = 0.0
		lib.MainNode_brakeCommand(self.obj, value+updown)

	def throttleCommand(self, value):
		# We are using PS3 left joystick: so axis (0,1) run in pairs, left/right for 2, up/down for 3
		# Change this if you want to switch to another axis on your joystick!
		# Normally they are centered on (0,0)
		updown = joystick.get_axis(1)
		if updown < 0.0:
			updown = 0.0
		lib.MainNode_throttleCommand(self.obj, value+updown)

	def connectPolySync(self):
		lib.MainNode_connectPolySync(self.obj)
