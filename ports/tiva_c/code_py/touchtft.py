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
		tft.fill(y,x,20,20,0x0000)
		x=xpt2046.getX()
		y=xpt2046.getY()
		tft.fill(y,x,20,20,0xff80)
		print("z:", z)
		print("x:", x)
		print("y:", y)
		switch1 = gpio.read(gpio.sw1)
		if switch1 == 0:
			exit = 1
		time.sleep_ms(500)
