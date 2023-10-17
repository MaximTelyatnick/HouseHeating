[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/MaximTelyatnick/HouseHeating?_x_tr_sl=ru&_x_tr_tl=en)  

Arduino Sketch Folder
=====================
Принципова схема
--------
Деталі
--------
* `arduino/flight_telemetry` Flight Telemetry for Project SAPHE
  * Arduino MEGA 2560
  * [Sparkfun] (http://www.sparkfun.com/products/9694) BMP085 Barometric Pressure / Temperature / Altitude sensor
  * [Adafruit] (http://www.adafruit.com/products/243) Datalogger Shield
  * [Adafruit] (https://www.adafruit.com/products/757) logic level converter
  * [Adafruit] (https://www.adafruit.com/products/372) 10k thermistor
  * [Adafruit] (https://www.adafruit.com/products/746) Ultimate GPS (high altitude?)
  * [Adafruit] (https://www.adafruit.com/products/358) 1.8" tft lcd

* `arduino/reflow_oven` Solder reflow oven controller
	* [Black and Decker] (http://www.amazon.com/Black-Decker-FC300-Infrawave-Oven/dp/B001AX8F4Y/ref=sr_1_2?ie=UTF8&qid=1336673199&sr=8-2) Infrawave Oven
	* [Adafruit] (http://www.adafruit.com/products/269) MAX6675 Thermocouple Amplifier
	* [Adafruit] (http://www.adafruit.com/products/270) Thermocouple Type K
	* [Sparkfun] (http://www.sparkfun.com/products/709) 16x2 LCD (HD44780 chipset)


Библиотеки
---------
* `arduino/libraries/U8glib` [LINK](https://www.arduino.cc/reference/en/libraries/u8glib/) Библиотека для работы с LCD дисплеем (в нашем варианте по шине I2C)
* `arduino/libraries/Keypad`[LINK](https://playground.arduino.cc/Code/Keypad/) Библиотека для упрощения работы с матричной клавиатурой
* `arduino/libraries/DallasTemperature`[LINK](https://playground.arduino.cc/Code/Timer1/) Библиотека для измерения температуры при помощи датчика DS18B20 
* `arduino/libraries/TimerOne`[LINK](https://playground.arduino.cc/Code/Timer1/) Библиотека для реализации многопоточности
* `arduino/libraries/PWMrelay` [LINK](https://github.com/GyverLibs/PWMrelay) Библиотека для управления нагрузкой на твердотельном реле 
