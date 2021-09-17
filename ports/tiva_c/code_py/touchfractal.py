import tft
import xpt2046
import time
import fractals
import gpio
gpio.input(gpio.sw1)
xpt2046.setRotation(3)
def calib():
	tft.clear(0)
	x = 240
	y = 160
	exit = 5
	while exit>0:
		z=xpt2046.getZ()
		if z > 10:
			tft.fill(x,y,20,20,0x0000)
			x=xpt2046.getX()
			y=xpt2046.getY()
			tft.fill(x,y,20,20,0xff80)
			print("z:", z)
			print("x:", x)
			print("y:", y)
			exit = exit - 1
		else:
			tft.fill(x,y,20,20,0xffff)
		time.sleep_ms(250)
def frac(iteration):
	exit=0
	x1=-60000
	x2=18000
	y1=-38000
	y2=38000
	x=240
	y=160
	while exit==0:
		fractals.set_denominators(32000,32000,32000,32000)
		fractals.plot_mandel(x1,x2,y1,y2,iteration)
		exit2=5
		switch1 = gpio.read(gpio.sw1)
		if switch1 == 0: exit = 1
		while exit2>0 and exit == 0:
			z=xpt2046.getZ()
			if z > 10:
				tft.fill(x,y,6,6,0x0000)
				x=xpt2046.getX()
				y=xpt2046.getY()
				tft.fill(x,y,6,6,0xff80)
				exit2 = exit2 - 1
			else:
				exit2 = 5
				tft.fill(x,y,6,6,0xffff)
			time.sleep_ms(300)
			if exit2==0:
				width = (x2-x1)//2
				height = (y2-y1)//2
				x1 = x1 + width*x//480
				y1 = y1 + height*(360-y)//360
				x2 = x1 + width
				y2 = y1 + height
				print ('X1:',x1)
				print ('W :',width)
				print ('Y1:',y1)
				print ('H :',height)
		if width < 1 or height < 1: exit = 1

		
	
	
		
	
