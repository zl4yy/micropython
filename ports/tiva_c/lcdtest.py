print('Frozen Python to test LCD 5110.\n')
import lcd
import time
lcd.init()
time.init()
lcd.setfont(lcd.large)
lcd.text(0,0,'TIVA C')
lcd.setfont(lcd.small)
lcd.text(0,2,'MicroPython')
lcd.text(0,3,'LCD 5110')
exit = 3
while exit > 0:
    lcd.inversevideo(1)
    time.sleep_ms(500)
    lcd.inversevideo(0)
    time.sleep_ms(500)
    exit = exit - 1
