print('Frozen Python to test GPIO.\n Press board switches to change LED colour.\n')
import gpio
gpio.init()
gpio.output(gpio.red)
gpio.output(gpio.green)
gpio.output(gpio.blue)
gpio.input(gpio.sw1)
gpio.input(gpio.sw2)
gpio.up(gpio.red)
exit = 0
while exit == 0:
	switch1 = gpio.read(gpio.sw1)
	switch2 = gpio.read(gpio.sw2)
	if switch1 == 0 and switch2 == 0:
		exit = 1;
	elif switch1 == 0:
		gpio.up(gpio.green)
		gpio.down(gpio.red)
		gpio.down(gpio.blue)
	elif switch2 == 0:
		gpio.up(gpio.blue)
		gpio.down(gpio.green)
		gpio.down(gpio.red)
	else:
		gpio.up(gpio.red)
		gpio.down(gpio.green)
		gpio.down(gpio.blue)
print('Loop ended')
gpio.down(gpio.red)
gpio.down(gpio.green)
gpio.down(gpio.blue)
