[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/MaximTelyatnick/HouseHeating?_x_tr_sl=ru&_x_tr_tl=en) 
[![Foo](https://img.shields.io/badge/README-UKRAINE-blue.svg?style=flat-square)](https://github-com.translate.goog/MaximTelyatnick/HouseHeating?_x_tr_sl=ru&_x_tr_tl=uk) 


Принципиальная схема
--------

<div id="top"></div>
<div align="center">
  <a href="https://github.com/MaximTelyatnick/HouseHeating">
    <img src="docs/chema.jpg" alt="Принципиальная схема" width="640" height="480">
  </a>
</div>
<h3 align="center">Принципиальная схема</h1>
<br> 

| Элемент     | Назначение | Устройство |
| ---      | ---  | ---  |
| LED1 | Используется для отображения информации и выбора режима работы | LCD 2004 SPI |
| DS1    | Герметичный датчик температуры DS18B20, используется для измерения температуры котла        |
| DS2 | Герметичный датчик температуры DS18B20, используется для измерения температуры теплового аккумулятора        |
| DS3 | герметичный датчик температуры DS18B20, используется для измерения температуры теплового аккумулятора         |
| DS4 | `         |
| KEY1 | `         |
| R1 | `         |
| R2 | `         |
| CONTROLLER1 | `         |
| REL1 | `         |
| REL2 | `         |
| REL3 | `         |
| REL4 | `         |
| REL5 | `         |
| REL6 | `         |

Детали
--------
* Arduino Mega 2560 Rev3 [LINK](https://arduino.ua/ru/prod243-arduino-mega-2560-rev3) Плата управления
* LCD 2004 I2C [LINK](https://arduino.ua/ru/prod1932-lcd-2004-i2c-simvolnii-displei-20x4-jeltii) Экран для отображения информации
* DS18B20 [LINK](https://arduino.ua/ru/prod414-temperatyrnii-datchik-vodonepronicaemii-ds18b20) Водонипроницаемый датчик измерения температуры
* 8-ми канальный модуль твердотельного реле 5В 2А [LINK]([https://www.adafruit.com/products/757](https://arduino.ua/ru/prod1415-8-mi-kanalnii-modyl-tverdotelnogo-rele-5v-2a-low-level)) Твердотельное реле для управления нагрузкой
* Клавиатура матричная 4х4 [LINK](https://arduino.ua/ru/prod316-klaviatyra-matrichnaya-4h4) Клавиатура для ввода данных и управления системой
* Подстроечный резистор 3296W (1 кОм) [LINK](https://arduino.ua/ru/prod5553-podstroechnii-rezistor-3296w-1-kom-1sht) Обвязка


Библиотеки
---------
* `arduino/libraries/U8glib` [LINK](https://www.arduino.cc/reference/en/libraries/u8glib/) Библиотека для работы с LCD дисплеем (в нашем варианте по шине I2C)
* `arduino/libraries/Keypad`[LINK](https://playground.arduino.cc/Code/Keypad/) Библиотека для упрощения работы с матричной клавиатурой
* `arduino/libraries/DallasTemperature`[LINK](https://playground.arduino.cc/Code/Timer1/) Библиотека для измерения температуры при помощи датчика DS18B20 
* `arduino/libraries/TimerOne`[LINK](https://playground.arduino.cc/Code/Timer1/) Библиотека для реализации многопоточности
* `arduino/libraries/PWMrelay` [LINK](https://github.com/GyverLibs/PWMrelay) Библиотека для управления нагрузкой на твердотельном реле 
