print('Test LCD and MMA7455.\n')
import time
import lcd
import mma7455
import gpio
gpio.init()
lcd.init()
gpio.output(gpio.red)
gpio.input(gpio.sw1)
gpio.input(gpio.sw2)
mma7455.init(0,2)
lcd.setfont(1)
lcd.backlight(1)
exit = 0
while exit == 0:
	lcd.clear(0)
	gpio.up(gpio.red)
	time.sleep_ms(50)
	x = mma7455.get_x()
	lcd.clear(0)
	lcd.text(0,0,str(x//100))
	lcd.text(2,0,'.')
	lcd.text(4,0,str((x%100)//10))
	lcd.text(6,0,str(x%10))
	time.sleep_ms(50)
	y = mma7455.get_y()
	lcd.text(0,2,str(y//100))
	lcd.text(2,2,'.')
	lcd.text(4,2,str((y%100)//10))
	lcd.text(6,2,str(y%10))
	time.sleep_ms(50)
	z = mma7455.get_z()
	lcd.text(0,4,str(z//100))
	lcd.text(2,4,'.')
	lcd.text(4,4,str((z%100)//10))
	lcd.text(6,4,str(z%10))
	gpio.down(gpio.red)
	i = 0
	while i<180:
		switch1 = gpio.read(gpio.sw1)
		time.sleep_ms(10)
		i = i + 1
		if switch1 == 0:
			i=100
			exit = 1		
lcd.backlight(0)
print('Loop ended')
