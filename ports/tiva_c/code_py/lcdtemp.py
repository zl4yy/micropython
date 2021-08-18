print('Test LCD and BMP085.\n')
import time
import lcd
import bmp085
import gpio
gpio.init()
time.init()
lcd.init()
gpio.output(gpio.red)
gpio.input(gpio.sw1)
gpio.input(gpio.sw2)
bmp085.init(0)
bmp085.setoss(3)
lcd.setfont(1)
lcd.backlight(1)
exit = 0
print('Press SW1 to end.\n')
while exit == 0:
	gpio.up(gpio.red)
	temp = bmp085.get_temp()
	lcd.text(0,0,str(temp//10))
	lcd.text(4,0,'.')
	lcd.text(6,0,str(temp%10))
	pressure = bmp085.get_pressure()
	if pressure < 100000:	
		lcd.text(0,3,str(pressure//100))
		lcd.text(6,3,'.')
		lcd.text(8,3,str((pressure%100)//10))
		lcd.text(10,3,str(pressure%10))
	else:
		lcd.text(0,3,str(pressure//100))
		lcd.text(8,3,'.')
		lcd.text(10,3,str((pressure%100)//10))
		lcd.text(12,3,str(pressure%100))
	lcd.setfont(0)
	lcd.text(0,2,'Deg C')
	lcd.text(0,5,'hPa')
	lcd.setfont(1)	
	gpio.down(gpio.red)
	i = 0
	while i<190:
		switch1 = gpio.read(gpio.sw1)
		time.sleep_ms(10)
		i = i + 1
		if switch1 == 0:
			i=100
			exit = 1		
lcd.backlight(0)
print('Loop ended')
