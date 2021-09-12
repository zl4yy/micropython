import tft
import xpt2046
import time
import gpio
import fractals
gpio.input(gpio.sw1)
gpio.input(gpio.sw2)
xpt2046.setRotation(3)
def calib():
	tft.clear(0)
	x = 240
	y = 160
	exit = 0
	while exit==0:
		z=xpt2046.getZ()
		# Do if z is touch for fill and x/y
		tft.fill(x,y,20,20,0x0000)
		x=xpt2046.getX()
		y=xpt2046.getY()
		tft.fill(x,y,20,20,0xff80)
		print(z)
		print(x)
		print(y)
		switch1 = gpio.read(gpio.sw1)
		if switch1 == 0:
			exit = 1
		time.sleep_ms(500)
def frac(iteration):
	exit=0
	zoom=1
	x1=-32000
	x2=16000
	y1=-18000
	y2=18000
	x=240
	y=160
	while exit==0:
		fractals.set_denominators(16000,16000,16000,16000)
		fractals.plot_mandel(x1,x2,y1,y2,iteration)
		exit2=0
		while exit2==0:
			switch1 = gpio.read(gpio.sw1)
			z=xpt2046.getZ()
			tft.fill(x,y,6,6,0x0000)
			x=xpt2046.getX()
			y=xpt2046.getY()
			tft.fill(x,y,6,6,0xffff)
			time.sleep_ms(400)
			if switch1==0:
				width = (x2-x1)//2
				height = (y2-y1)//2
				x1 = x1 + width*x//480
				y1 = y1 + height*(360-y)//360
				x2 = x1 + width
				y2 = y1 + height
				zoom=zoom+1
				print ('X1:',x1)
				print ('X2:',x2)
				print ('Y1:',y1)
				print ('Y2:',y2)
				exit2=1
		switch2 = gpio.read(gpio.sw2)
		if switch2 == 0:
			exit = 1
	
	
		
	
