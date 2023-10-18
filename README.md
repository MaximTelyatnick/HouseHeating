[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/MaximTelyatnick/HouseHeating?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/README-UKRAINE-blueyellow.svg?style=flat-square)](https://github-com.translate.goog/MaximTelyatnick/HouseHeating?_x_tr_sl=ru&_x_tr_tl=uл) 

Arduino Sketch Folder
=====================
Принципова схема
--------
Детали
--------
* `arduino/flight_telemetry` Flight Telemetry for Project SAPHE
  * `Arduino Mega 2560 Rev3` [LINK] (https://arduino.ua/ru/prod243-arduino-mega-2560-rev3) Плата управления
  * `LCD 2004 I2C` [LINK](https://arduino.ua/ru/prod1932-lcd-2004-i2c-simvolnii-displei-20x4-jeltii) Экран для отображения информации
  * `DS18B20` [LINK]([http://www.adafruit.com/products/243](https://arduino.ua/ru/prod414-temperatyrnii-datchik-vodonepronicaemii-ds18b20)) Водонипроницаемый датчик температуры
  * `8-ми канальный модуль твердотельного реле 5В 2А` [LINK]([https://www.adafruit.com/products/757](https://arduino.ua/ru/prod1415-8-mi-kanalnii-modyl-tverdotelnogo-rele-5v-2a-low-level)) logic level converter
  * `Клавиатура матричная 4х4` [LINK](https://arduino.ua/ru/prod316-klaviatyra-matrichnaya-4h4) 10k thermistor
  * `Подстроечный резистор 3296W (1 кОм)` [LINK](https://arduino.ua/ru/prod5553-podstroechnii-rezistor-3296w-1-kom-1sht) Ultimate GPS (high altitude?)

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
