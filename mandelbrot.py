import cv2
import numpy as np
from threading import Thread

class Mandelbrot(object):
	
	def __init__(self):
		self.width = self.height = 800
		self.min_x = -2.0
		self.max_x = .5
		self.min_y = -1.25
		self.max_y = 1.25
		self.scale = .2
		self.x_step = (self.max_x - self.min_x)/self.width
		self.y_step = (self.max_y - self.min_y)/self.height
		self.max_iter = 500

	def to_coords(self,x,y):
		x_coord = self.min_x + x*self.x_step
		y_coord = self.min_y + (self.height-y)*self.y_step
		return (x_coord,y_coord)

	def zoom(self,event,x,y,flags,param):
		if event == cv2.EVENT_LBUTTONDOWN:
			(x_coord,y_coord) = self.to_coords(x,y)
			self.min_x = x_coord - self.scale
			self.max_x = x_coord + self.scale
			self.min_y = y_coord - self.scale
			self.max_y = y_coord + self.scale
			self.x_step = (self.max_x - self.min_x)/self.width
			self.y_step = (self.max_y - self.min_y)/self.height
			print(self.max_y)

	
	def mandelbrot(self,x,y):
		z = c = complex(x,y)
		for n in range(self.max_iter):
			if abs(z) > 2:
				return n
			z = z*z + c
		return 255


	def thread_mandelbrot(self,num_threads):
		thrs = []
		img = np.zeros((self.width,self.height,3), np.uint8)
		for a in range(num_threads):
			thrs.append(Thread(target=self.do_mandelbrot_thread,args=(img,a,num_threads)))
			thrs[a].start()
		for a in thrs:
			a.join()
		return img

	def do_mandelbrot_thread(self,img,thread_id,num_threads):
		x_coord = self.min_x - self.x_step
		for x in range(self.width):
			x_coord += self.x_step
			y_coord = self.max_y + self.y_step - self.y_step*thread_id
			for y in range(thread_id,self.height,num_threads):
				y_coord -= self.y_step*num_threads
				it = self.mandelbrot(x_coord,y_coord)
				(r,g,b) = (it*8,it*4,it*2)
				if r > 255:
					r = 255
				if g > 255:
					g = 255
				if b > 255:
					b = 255
				img[y,x] = (b,g,r)
		return img

	def do_mandelbrot(self):
		img = np.zeros((self.width,self.height,3), np.uint8)
		x_coord = self.min_x - self.x_step
		for x in range(self.width):
			x_coord += self.x_step
			y_coord = self.max_y + self.y_step
			for y in range(self.height):
				y_coord -= self.y_step
				it = self.mandelbrot(x_coord,y_coord)
				(r,g,b) = (it*8,it*4,it*2)
				if r > 255:
					r = 255
				if g > 255:
					g = 255
				if b > 255:
					b = 255
				img[y,x] = (b,g,r)
		return img


m = Mandelbrot()
cv2.namedWindow('image')
cv2.setMouseCallback('image',m.zoom)



		
while True:
	img = m.do_mandelbrot()
	cv2.imshow('image',img)
	key = cv2.waitKey(0)
	m.scale = m.scale / 5.0
	if key == ord('q'):
		break
cv2.destroyAllWindows()
