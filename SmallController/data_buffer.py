import queue


class DataBuffer():

	SIZE = 2

	def __init__(self):
		self.queue = queue.Queue(maxsize=self.SIZE)

	def add_item(self, item):
		if self.queue.full():
			self.queue.get() # remove fist item from queue
			self.queue.put(item)
		else:
			self.queue.put(item)

	def get_item_for_processing(self):
		return self.queue.get()
		