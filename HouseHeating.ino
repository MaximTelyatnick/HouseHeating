#include "U8glib.h"
#include <Keypad.h>
#include <microDS18B20.h>
#include <MsTimer2.h>
#include <math.h>
#include <string.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include "DallasTemperature.h"
#include "PWMrelay.h"
#include "rus4x6.h"
#include "rus5x8.h"
#include "rus6x12.h"
#include "rus10x20.h"

//перечисленние использованніх портов
const uint8_t powerLoadPin = A3; // пин по которому проверяем есть ли питание в сети
short powerLoadCounter = 0; // счетчик для пониманния того что сеть точно пропала а не помеха
//перечисленние использованніх портов
const uint8_t temperatureBusPin = 10; // шина датчиков температуры
//порты дисплея
const uint8_t brightDisplayPin = 16; // подсветка дисплея
const uint8_t spiDisplayPin = 17; // подсветка дисплея
const uint8_t csDisplayPin = 18; // подсветка дисплея
const uint8_t mosiDisplayPin = 19; // подсветка дисплея

//указываем порты реле
const uint8_t boilerPumpRelayPin = A10; // реле насос котла
const uint8_t heatAccumPumpRelayPin = A9; // насос теплоаккумулятора
const uint8_t primaryCirculationPumpRelayPin = A8; // Насос циркуляции первичного круга
const uint8_t heatingCirculationPumpRelayPin = A7; // Насос циркуляции отопления
const uint8_t heatExchangerPumpRelayPin = A6; // Насос циркуляции подачи на теплообменник
const uint8_t heatWaterCirculationPumpRelayPin = A5; // Насос циркуляции горячей воды

PWMrelay boilerPumpRelay(boilerPumpRelayPin); // реле на 13 пине
PWMrelay heatAccumPumpRelay(heatAccumPumpRelayPin); // реле на 13 пине
PWMrelay primaryCirculationPumpRelay(primaryCirculationPumpRelayPin); // реле на 13 пине
PWMrelay heatingCirculationPumpRelay(heatingCirculationPumpRelayPin); // реле на 13 пине
PWMrelay heatExchangerPumpRelay(heatExchangerPumpRelayPin); // реле на 13 пине
PWMrelay heatWaterCirculationPumpRelay(heatWaterCirculationPumpRelayPin); // реле на 13 пине

//указание пинов для использования дисплея, не обязательно брать пины аппаратного SPI
U8GLIB_ST7920_128X64_1X u8g(spiDisplayPin, mosiDisplayPin, csDisplayPin); //SPI Com: SCK = en = spiDisplayPin, MOSI = rw = mosiDisplayPin, CS = di = csDisplayPin



// указываем адресса датчиков температуры
uint8_t boilerTempAddresSensor_addr[] = {0x28, 0xFF, 0x64, 0x1E, 0x5A, 0x7A, 0xB9, 0x85};
uint8_t heatAccumTempAddresSensor_addr[] = {0x28, 0xFF, 0x64, 0x1E, 0x5A, 0x69, 0x84, 0x7F};
uint8_t primaryRingTempAddresSensor_addr[] = {0x28, 0xFF, 0x64, 0x1E, 0x5A, 0x7F, 0xE2, 0x81};
uint8_t hotWaterTempAddresSensor_addr[] = {0x28, 0xFF, 0x64, 0x1E, 0x5B, 0x85, 0x58, 0x18};
// инициализация датчиков температуры
MicroDS18B20<temperatureBusPin, boilerTempAddresSensor_addr> boilerTempAddresSensor;  // Создаем термометр с адресацией
MicroDS18B20<temperatureBusPin, heatAccumTempAddresSensor_addr> heatAccumTempAddresSensor;  // Создаем термометр с адресацией
MicroDS18B20<temperatureBusPin, primaryRingTempAddresSensor_addr> primaryRingTempAddresSensor;  // Создаем термометр с адресацией
MicroDS18B20<temperatureBusPin, hotWaterTempAddresSensor_addr> hotWaterTempAddresSensor;  // Создаем термометр с адресацией


const byte ROWS = 4; // 4 строки
const byte COLS = 4; // 4 столбца
char keys[ROWS][COLS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] =  {37, 35, 39, 41};
byte colPins[COLS] =  {27, 29, 31, 33};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//Режимы


//Переменные отвечающие за меню
//Массив с названиями меню
char* MenuNames[50];
char* MenuNamesSecondString[50];
//Тип элемента меню
//0-родительское меню
//1-целое число
//2-временной интервал (h:m:s, целое число но отображается как время)
//3-Вкл/Выкл (целое число, но отображается как On/Off)
//4-Расстояние (cm, целое число, но отображается как 0.хх метров)
//5-Тип питания (целое число, но отображается как Сеть/Бат.)
//6-Вольтаж батарие (отображется как float, на самом деле отделяем последний символ точкой (для ускорения работы))
//7-Режим отопления (целое числ, но отображается как Эконом/Обычный)
//8-Отображение температуры (целове число, но отображается как число + 'С )
//9-Режим подогрева (целове число, но отображается Горячая вода/Басейн )
//10-Вывод минут  (целове число, но отображается Число + Мин. )
int MenuTypeCode[50];
int MenuTypeCodeSecond[50];
//Значение элемента меню
int MenuValue[50];
//Значение второго параметра меню
int MenuValueSecond[50];
//Текущая позиция в меню, вложенность не важна т.к. меню представляет из себя список с ссылками на родительские 
//и дочерние пункты
int MenuNowPos=0;
//Режим редактирования (0-нет, просто выделен определенный параметр или пункт меню, 1-редактируем значение параметра, 2-редактируем значение второго параметр)
int MenuEdit=0;
//номер элемента меню который является для данного родительским
//0-нет родительского элемента
int MenuParent[50];
//номер элемента меню который является для данного дочерним
//0-нет дочернего элемента
int MenuChild[50];
//Номер элемента дочернего меню который является первым
int MenuChildFirst[50];
//номер элемента дочернего меню который является последним
int MenuChildEnd[50];
//позиция меню в которой находится выделенный пункт на экране (например на экране отображается 3 пункта при этом выделен второй)
int MenuDrawPos=0;
//максимальное количество отображаемых на экране пунктов
int MenuDrawCount=7;


//переменные для анализа времени работы от аккумулятора
//время последней перерисовки
long PowerBatteryTime1=0;
//текущее время
long PowerBatteryTime2=0;


//переменные для считывания датчиков температуры
//время последнего опроса датчиков
long TempSensorTime1=0;
//текущее время
long TempSensorTime2=0;
//интервал для опроса датчиков
long TempSensorTimeInterval=2000000; //1 секунды


//переменные для таймера перерисовки
//время последней перерисовки
long DrawTime1=0;
//текущее время
long DrawTime2=0;
//интервал для перерисовки экрана
long DrawTimeInterval=100000; //0.1 секунды

//переменные для таймера перерисовки
//время последней перерисовки
long RelayTime1=0;
//текущее время
long RelayTime2=0;
//интервал для перерисовки экрана
long RelayTimeInterval=500000; //0.5 секунда

//переменные для контроля подсветки дисплея
//время последнего включения
long LedTime1=0;
//текущее время
long LedTime2=0;
//интервал для проверки
long LedTimeInterval=30000000; //30 секунд
//интервал для проверки
uint8_t LedFlag = 1; //30 секунд

//переменные для контроля реле в режиме єкономии
//время последнего включения
long RelayEconomTime1=0;
//текущее время
long RelayEconomTime2=0;
//интервал для проверки
long RelayEconomInterval=0; 
long RelayPowerInterval=0; 
//флаг
bool RalayFlag = false;


void MenuSetup()
{
//Настройка меню
//задаем начальное положение в меню
MenuNowPos=4;
MenuDrawPos=0;
//Массив с названиями меню, индексами родительских элементов меню, начальными и конечными индексами дочерних элементов меню
//также задаем начальные значения параметров элементов и их тип
MenuNames[0]="";
MenuTypeCode[0]=0;
MenuValue[0]=0;
MenuParent[0]=0;
MenuChildFirst[0]=1;
MenuChildEnd[0]=3;


MenuNames[1]="  Мониторинг  ";
MenuTypeCode[1]=0;
MenuValue[1]=0;
MenuParent[1]=0;
MenuChildFirst[1]=4;
MenuChildEnd[1]=9;


MenuNames[2]="  Настройки   ";
MenuTypeCode[2]=0;
MenuValue[2]=0;
MenuParent[2]=0;
MenuChildFirst[2]=10;
MenuChildEnd[2]=15;


MenuNames[3]="    Режимы    ";
MenuTypeCode[3]=0;
MenuValue[3]=0;
MenuParent[3]=0;
MenuChildFirst[3]=16;
MenuChildEnd[3]=19;


MenuNames[4]="Котел:";
MenuTypeCode[4]=3;
MenuValue[4]=1;
MenuTypeCodeSecond[4]=8;
MenuValueSecond[4]=20;
MenuParent[4]=1;
MenuChildFirst[4]=0;
MenuChildEnd[4]=0;

MenuNames[5]="Теп. акк.:";
MenuTypeCode[5]=3;
MenuValue[5]=1;
MenuTypeCodeSecond[5]=8;
MenuValueSecond[5]=20;
MenuParent[5]=1;
MenuChildFirst[5]=0;
MenuChildEnd[5]=0;

MenuNames[6]="Пер.кольцо:";
MenuTypeCode[6]=3;
MenuValue[6]=1;
MenuTypeCodeSecond[6]=8;
MenuValueSecond[6]=20;
MenuParent[6]=1;
MenuChildFirst[6]=0;
MenuChildEnd[6]=0;

MenuNames[7]="Отоплен.:";
MenuTypeCode[7]=3;
MenuValue[7]=1;
//MenuTypeCodeSecond[7]=8;
//MenuValueSecond[7]=20;
MenuParent[7]=1;
MenuChildFirst[7]=0;
MenuChildEnd[7]=0;

MenuNames[8]="Теп. обм.:";
MenuTypeCode[8]=3;
MenuValue[8]=1;
//MenuTypeCodeSecond[8]=8;
//MenuValueSecond[8]=20;
MenuParent[8]=1;
MenuChildFirst[8]=0;
MenuChildEnd[8]=0;

MenuNames[9]="Гор. вода:";
MenuTypeCode[9]=3;
MenuValue[9]=1;
MenuTypeCodeSecond[9]=8;
MenuValueSecond[9]=20;
MenuParent[9]=1;
MenuChildFirst[8]=0;
MenuChildEnd[9]=0;

MenuNames[10]="   Порог отключения   ";
MenuNamesSecondString[10]="     насоса котла     ";
MenuTypeCode[10]=8;
MenuValue[10]=50;
MenuParent[10]=2;
MenuChildFirst[10]=0;
MenuChildEnd[10]=0;

MenuNames[11]=" Минамальная рабочая  ";
MenuNamesSecondString[11]="     температура      ";
MenuTypeCode[11]=8;
MenuValue[11]=45;
MenuParent[11]=2;
MenuChildFirst[11]=0;
MenuChildEnd[11]=0;

MenuNames[12]=" Разница температур  ";
MenuNamesSecondString[12]="    горячей воды     ";
MenuTypeCode[12]=8;
MenuValue[12]=5;
MenuParent[12]=2;
MenuChildFirst[12]=0;
MenuChildEnd[12]=0;

MenuNames[13]="Время работы в режиме";
MenuNamesSecondString[13]="      экономии       ";
MenuTypeCode[13]=10;
MenuValue[13]=3;
MenuParent[13]=2;
MenuChildFirst[13]=0;
MenuChildEnd[13]=0;

MenuNames[14]="Время простоя в режиме";
MenuNamesSecondString[14]="       экономии      ";
MenuTypeCode[14]=10;
MenuValue[14]=5;
MenuParent[14]=2;
MenuChildFirst[14]=0;
MenuChildEnd[14]=0;

MenuNames[15]="Температура включения ";
MenuNamesSecondString[15]="  аварийного режима   ";
MenuTypeCode[15]=8;
MenuValue[15]=5;
MenuParent[15]=2;
MenuChildFirst[15]=0;
MenuChildEnd[15]=0;

MenuNames[16]="       Питание        ";
MenuTypeCode[16]=5;
MenuValue[16]=0;
MenuTypeCodeSecond[16]=8;
MenuValueSecond[16]=20;
MenuParent[16]=3;
MenuChildFirst[16]=0;
MenuChildEnd[16]=0;

            MenuNames[17]="   Напряжение порта   ";
MenuNamesSecondString[17]="     наличия сети     ";
MenuTypeCode[17]=6;
MenuValue[17]=122;

MenuParent[17]=3;
MenuChildFirst[17]=0;
MenuChildEnd[17]=0;

MenuNames[18]="   Режим отопления    ";
MenuTypeCode[18]=7;
MenuValue[18]=0;
MenuParent[18]=3;
MenuChildFirst[18]=0;
MenuChildEnd[18]=0;

MenuNames[19]="   Режим подогрева    ";
MenuTypeCode[19]=9;
MenuValue[19]=0;
MenuParent[19]=3;
MenuChildFirst[19]=0;
MenuChildEnd[19]=0;


//считываем параметры из памяти
int i=0;
for (i=0;i<50;i++){
  MenuValue[i]=(EEPROM.read(i*2-1) << 8);
  MenuValue[i]=MenuValue[i]+(EEPROM.read(i*2-2));
  }
}


void setup()
{
  //Timer1.initialize(200);// инициализация таймера 1, период 250 мкс
  //myPID.SetOutputLimits(0, 250);
 // Timer1.attachInterrupt(timerInterruptExtruder, 200);  // задаем обработчик прерываний
  //Timer1.attachInterrupt(timerInterruptMoved, 200);

  // апаратные настройки реле
  
  PWMrelay boilerPumpRelay(boilerPumpRelayPin); // 1 реле 
  boilerPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  boilerPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  boilerPumpRelay.setPWM(255);
  
  PWMrelay heatAccumPumpRelay(heatAccumPumpRelayPin); // 2 реле
  heatAccumPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  heatAccumPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  heatAccumPumpRelay.setPWM(255);
  
  PWMrelay primaryCirculationPumpRelay(primaryCirculationPumpRelayPin); // 3 реле
  heatAccumPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  heatAccumPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  heatAccumPumpRelay.setPWM(255);
  
  PWMrelay heatingCirculationPumpRelay(heatingCirculationPumpRelayPin); // 4 реле
  primaryCirculationPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  primaryCirculationPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  primaryCirculationPumpRelay.setPWM(255);
  
  PWMrelay heatExchangerPumpRelay(heatExchangerPumpRelayPin); // 5 реле
  heatExchangerPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  heatExchangerPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  heatExchangerPumpRelay.setPWM(255);
  
  PWMrelay heatWaterCirculationPumpRelay(heatWaterCirculationPumpRelayPin); // 6 реле
  heatWaterCirculationPumpRelay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW) 
  heatWaterCirculationPumpRelay.setPeriod(1000);  // можно поменять период, миллисекунды
  heatWaterCirculationPumpRelay.setPWM(255);
  
    
  
  /*pinMode(boilerPumpRelayPin,OUTPUT); //назначаем порт как "выход"
  pinMode(heatAccumPumpRelayPin,OUTPUT); //назначаем порт как "выход"
  digitalWrite(heatAccumPumpRelayPin, LOW);
  pinMode(primaryCirculationPumpRelayPin,OUTPUT); //назначаем порт как "выход"
  pinMode(heatingCirculationPumpRelay,OUTPUT); //назначаем порт как "выход"
  pinMode(heaExchangerPumpRelayPin,OUTPUT); //назначаем порт как "выход"
  pinMode(heatWaterCirculationPumpRelayPin,OUTPUT); //назначаем порт как "выход"*/
  // апаратные настройки порта питания
  pinMode(powerLoadPin,INPUT_PULLUP); //назначаем порт как "вход"
  // апаратные настройки дисплея
  pinMode(brightDisplayPin,OUTPUT); //назначаем порт как "выход"
  digitalWrite(brightDisplayPin, HIGH); //включаем подсветку
  //настройка порта для клавиатуры
  keypad.setHoldTime(1000);
  keypad.setDebounceTime(50); 
  keypad.addEventListener(keypadEvent);

  // инициализация монитора последовательного порта (для отладки)
  Serial.begin(9600);

  
  
  //настройка дисплея
  u8g.setHardwareBackup(u8g_backup_avr_spi);
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);       
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);       
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  //формирование меню
  MenuSetup();
}




// str - передаваемая строка, fontSize - размер шрифта в пикселях, fontType - типа шрифта (почему-то русские символы воспринимаются как 2 байта)
int startStringPosistion(String str, int fontSize, int fontType = 1)
{
  int strLenght = str.length();
  int stringPosition = (int)(128/2)-((int)((strLenght*fontSize)/(2*fontType)));
  return stringPosition;  
}

//метод выводит "минут" относительно падежа
String minuteCase(int number)
{
  switch(number)
  {
    case 1:
      return (" минута");
      break;
    case 2:
    case 3:
    case 4:
      return (" минуты");
      break;
    default:
      return (" минут");
      break;        
  }
}

//метод выводит "минут" относительно падежа
void GetTempValue()
{
  boilerTempAddresSensor.requestTemp();      // Запрашиваем преобразование температуры
  heatAccumTempAddresSensor.requestTemp();
  primaryRingTempAddresSensor.requestTemp();      // Запрашиваем преобразование температуры
  hotWaterTempAddresSensor.requestTemp();
  MenuValueSecond[4] = boilerTempAddresSensor.getTempInt();
  MenuValueSecond[5] = heatAccumTempAddresSensor.getTempInt();
  MenuValueSecond[6] = primaryRingTempAddresSensor.getTempInt();
  MenuValueSecond[9] = hotWaterTempAddresSensor.getTempInt();
  
}




void DrawSinglePageMenu()
{
  int DrawPosistionYCoordinateText=8;
  int DrawPosistionXCoordinateText=0;
  String showValue = "";

  u8g.setFont(rus6x12);
  u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText);
  
  //u8g.print(MenuNowPos);
  //u8g.setPrintPos(20, 20);
  u8g.print(MenuNames[MenuNowPos]);
  DrawPosistionYCoordinateText+=DrawPosistionYCoordinateText+4;
  u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText);
  u8g.print(MenuNamesSecondString[MenuNowPos]);
  u8g.setFont(rus10x20);
  DrawPosistionYCoordinateText+=24;
  DrawPosistionXCoordinateText+=64;
  u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText);
  //u8g.print("10");
  //int keyForFirstParam = MenuTypeCode[MenuNowPos];

  
  int DrawHours=0;
  int DrawMinutes=0;
  int DrawSeconds=0;

  int typeMenuValue = MenuTypeCode[MenuNowPos];

  if(typeMenuValue ==1){
    showValue = (String)MenuValue[MenuNowPos];
    u8g.setPrintPos(startStringPosistion(showValue, 10), DrawPosistionYCoordinateText);
    u8g.print(showValue);
  }
  else if(typeMenuValue ==2){
    u8g.setFont(rus6x12);
    DrawHours=MenuValueSecond[MenuNowPos] / 3600;
    DrawMinutes=(MenuValueSecond[MenuNowPos] % 3600) / 60;
    DrawSeconds=(MenuValueSecond[MenuNowPos] % 3600) % 60;    
    u8g.print((String)DrawHours+":"+(String)DrawMinutes+":"+(String)DrawSeconds);
  }
  else if(typeMenuValue ==3)
  {
    if (MenuValue[MenuNowPos]==0) 
    {
      showValue = "Выкл";
      u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
      u8g.print(showValue);
    }
    else 
      {
        showValue = "Вкл";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
  }
  else if(typeMenuValue==4)        
  {
  }
  else if(typeMenuValue ==5)        
  {
    if (MenuValue[MenuNowPos]==0) 
      {
        showValue = "Сеть";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
      else 
      {
        showValue = "Аккумулятор";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
        MenuValueSecond[MenuNowPos]= (PowerBatteryTime2-PowerBatteryTime1)/1000;
        u8g.setFont(rus6x12);
        DrawHours=MenuValueSecond[MenuNowPos] / 3600;
        DrawMinutes=(MenuValueSecond[MenuNowPos] % 3600) / 60;
        DrawSeconds=(MenuValueSecond[MenuNowPos] % 3600) % 60;
        showValue = (String)DrawHours+":"+(String)DrawMinutes+":"+(String)DrawSeconds;
        u8g.setPrintPos(startStringPosistion(showValue, 6), DrawPosistionYCoordinateText+16);    
        u8g.print(showValue);
        
      }
  }
  else if(typeMenuValue ==6)        
  {
    showValue = (String)MenuValue[MenuNowPos];
    String s2 = showValue.substring(0,showValue.length()-1) + "." + showValue.substring(showValue.length()-1) + "V";
    u8g.setPrintPos(startStringPosistion(s2, 10), DrawPosistionYCoordinateText);
    u8g.print(s2);
  }
  else if(typeMenuValue ==7)        
  {
    if (MenuValue[MenuNowPos]==0) 
      {
        showValue = "Экономия";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
    else if (MenuValue[MenuNowPos]==1)
      {
        showValue = "Обычный";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
    else
      {
        showValue = "Отключено";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
  }
  else if(typeMenuValue ==8)        
  {
    //u8g.print(MenuValue[MenuNowPos]);
    //Serial.println(typeMenuValue);
  //°C
    showValue = (String)MenuValue[MenuNowPos]+(String)".C";
    u8g.setPrintPos(startStringPosistion(showValue, 10), DrawPosistionYCoordinateText);
    u8g.print(MenuValue[MenuNowPos]);
    //переходим на англоязычную кодировку чтобы вывести символ градуса
    u8g.setFont(u8g_font_10x20);
    u8g.print("\260C");
  }
  else if(typeMenuValue ==9)        
  {
    if (MenuValue[MenuNowPos]==0) 
      {
        showValue = "Горячая вода";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
    else if (MenuValue[MenuNowPos]==1) 
      {
        showValue = "Бассейн";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
    else
      {
        showValue = "Отключено";
        u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
        u8g.print(showValue);
      }
  }
  else if(typeMenuValue ==10)        
  {
    showValue = (String)MenuValue[MenuNowPos]+minuteCase(MenuValue[MenuNowPos]);
    u8g.setPrintPos(startStringPosistion(showValue, 10, 2), DrawPosistionYCoordinateText);
    u8g.print(showValue);
  }


  if (MenuEdit==1)
    { 
      u8g.drawFrame(5, 25, 117, 25);
    }
    
}


void DrawMenuFirstValue(int typeMenuValue, int DrawI, int DrawPosistionYCoordinateText)
{
  
  //временные переменные для отображения временных параметров 
  int DrawHours=0;
  int DrawMinutes=0;
  int DrawSeconds=0;
  
  switch (typeMenuValue){
          case 1:
            u8g.print(MenuValue[MenuNowPos-MenuDrawPos+DrawI]);
            break;
          case 2:
            DrawHours=MenuValue[MenuNowPos-MenuDrawPos+DrawI] / 3600;
            DrawMinutes=(MenuValue[MenuNowPos-MenuDrawPos+DrawI] % 3600) / 60;
            DrawSeconds=(MenuValue[MenuNowPos-MenuDrawPos+DrawI] % 3600) % 60;
            u8g.print((String)DrawHours+":"+(String)DrawMinutes+":"+(String)DrawSeconds);
            break;
          case 3:
            if (MenuValue[MenuNowPos-MenuDrawPos+DrawI]==0) {u8g.print("Выкл");}
              else {u8g.print("Вкл");}
            break;
          case 4:
            break;
          case 5:
            if (MenuValue[MenuNowPos-MenuDrawPos+DrawI]==0) {u8g.print("Сеть");}
              else {u8g.print("Аккум");}
            break;
          case 6:
            break;
          case 7:
            if (MenuValue[MenuNowPos-MenuDrawPos+DrawI]==1) {u8g.print("Норм.");}
              else {u8g.print("Эконом");}
            break;
          case 8:
            u8g.print(MenuValue[MenuNowPos-MenuDrawPos+DrawI]);
            u8g.setFont(u8g_font_6x12);
            u8g.print("\260C");
            u8g.setFont(rus6x12);
            
            break;
          default:
            break;
                     
        }
}

void DrawMenuSecondValue(int typeMenuValue, int DrawI, int DrawPosistionYCoordinateText)
{
  
  //временные переменные для отображения временных параметров 
  int DrawHours=0;
  int DrawMinutes=0;
  int DrawSeconds=0;
  
  switch (typeMenuValue){
          case 1:
            u8g.print(MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI]);
            break;
          case 2:
            DrawHours=MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI] / 3600;
            DrawMinutes=(MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI] % 3600) / 60;
            DrawSeconds=(MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI] % 3600) % 60;
            u8g.print((String)DrawHours+":"+(String)DrawMinutes+":"+(String)DrawSeconds);
            break;
          case 3:
            if (MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI]==0) {u8g.print("Выкл");}
              else {u8g.print("Вкл");}
            break;
          case 4:
            break;
          case 5:
            if (MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI]==0) {u8g.print("Сеть");}
              else {u8g.print("Аккум");}
            break;
          case 6:
            break;
          case 7:
            if (MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI]==1) {u8g.print("Норм.");}
              else {u8g.print("Эконом");}
            break;
          case 8:
            u8g.print(MenuValueSecond[MenuNowPos-MenuDrawPos+DrawI]);
            u8g.setFont(u8g_font_6x12);
            u8g.print("\260C");
            u8g.setFont(rus6x12);
            
            break;
          default:
            break;
                     
        }
}

void DrawMenu()
{
  if(MenuNowPos>3)
  {
    int DrawPosistionYCoordinateText=9;
    int DrawPosistionXCoordinateText=5;
    //размер шрифта
    u8g.setFont(rus6x12);
    //u8g.setFont(u8g_font_fixed_v0);  
    //вывод названия родительского меню вверху экрана
    u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText);
    /*if(MenuNames[MenuParent[MenuNowPos-MenuDrawPos]]!=""){
      u8g.print(MenuNames[MenuParent[MenuNowPos-MenuDrawPos]]);
      u8g.drawLine( 0, 10, 123,10);
      DrawPosistionYCoordinateText = 20;
    }*/
  
    //переменная для вывода пунктов меню на экран
    int DrawI=0;
    //цикл для вывода пунктов меню на экран
    for(DrawI=0; DrawI<MenuDrawCount;DrawI++)  
    {
      DrawPosistionXCoordinateText=5;
      u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText+9*DrawI);
      if ((MenuChildFirst[MenuParent[MenuNowPos]]<=(MenuNowPos-MenuDrawPos+DrawI)) and 
        (MenuChildEnd[MenuParent[MenuNowPos]]>=(MenuNowPos-MenuDrawPos+DrawI)))
        { 
  
          u8g.print(MenuNames[MenuNowPos-MenuDrawPos+DrawI]);
            
          u8g.setPrintPos(DrawPosistionXCoordinateText+75, DrawPosistionYCoordinateText+9*DrawI);
          int keyForFirstParam = MenuTypeCode[MenuNowPos-MenuDrawPos+DrawI];
          DrawMenuFirstValue(keyForFirstParam, DrawI, DrawPosistionYCoordinateText);
              
          u8g.setPrintPos(DrawPosistionXCoordinateText+100, DrawPosistionYCoordinateText+9*DrawI);  
          int keyForSecondParam = MenuTypeCodeSecond[MenuNowPos-MenuDrawPos+DrawI];
          DrawMenuSecondValue(keyForSecondParam, DrawI, DrawPosistionYCoordinateText);               
        }
    }
  }
  else
  {
    int DrawPosistionYCoordinateText=20;
    int DrawPosistionXCoordinateText=0;
    //размер шрифта
    u8g.setFont(rus10x20);
    //u8g.setFont(u8g_font_fixed_v0);  
    //вывод названия родительского меню вверху экрана
    u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText);
    /*if(MenuNames[MenuParent[MenuNowPos-MenuDrawPos]]!=""){
      u8g.print(MenuNames[MenuParent[MenuNowPos-MenuDrawPos]]);
      u8g.drawLine( 0, 10, 123,10);
      DrawPosistionYCoordinateText = 20;
    }*/
  
    //переменная для вывода пунктов меню на экран
    int DrawI=0;
    //цикл для вывода пунктов меню на экран
    for(DrawI=0; DrawI<MenuDrawCount;DrawI++)  
    {
      DrawPosistionXCoordinateText = 0;
      u8g.setPrintPos(DrawPosistionXCoordinateText, DrawPosistionYCoordinateText+18*DrawI);
      if ((MenuChildFirst[MenuParent[MenuNowPos]]<=(MenuNowPos-MenuDrawPos+DrawI)) and 
        (MenuChildEnd[MenuParent[MenuNowPos]]>=(MenuNowPos-MenuDrawPos+DrawI)))
            u8g.print(MenuNames[MenuNowPos-MenuDrawPos+DrawI]);
    }

    if (MenuEdit==0)
    { 
      u8g.drawFrame(0, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos, 127, 20); 
      /*if(migator>=0 && migator<=30){
        u8g.drawFrame(0, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos, 127, 20); 
        //u8g.drawLine( 0, (DrawPosistionYCoordinateText+5)+18*MenuDrawPos, 127, (DrawPosistionYCoordinateText+5)+18*MenuDrawPos);
        //u8g.drawLine( 0, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos, 127, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos);
        //u8g.drawLine( 0, (DrawPosistionYCoordinateText+5)+18*MenuDrawPos, 0, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos);
        //u8g.drawLine( 127, (DrawPosistionYCoordinateText+5)+18*MenuDrawPos, 127, (DrawPosistionYCoordinateText-15)+18*MenuDrawPos);
      }
      else if(migator>30)
      {
        u8g.drawFrame(0,0,0,0);
        migator=-50;
      }
      ++migator;*/
    }
  }
  
  //если параметр сейчас не редактируется то отображение рамки вокруг выделенного пункта меню
  
    
    
  //если параметр сейчас редактируется то отображение рамки вокруг значения параметра
  /*if (MenuEdit==1)
    {
      
      u8g.drawLine( 75, 12+7*MenuDrawPos, 122, 12+7*MenuDrawPos);
      u8g.drawLine(122, 12+7*MenuDrawPos, 122,22+7*MenuDrawPos);
      u8g.drawLine( 75,22+7*MenuDrawPos, 122,22+7*MenuDrawPos);
      u8g.drawLine( 75,  22+7*MenuDrawPos, 75,12+7*MenuDrawPos);
    }  
    */
}


/*void DrawMenu()
{
//временные переменные для отображения временных параметров  
int DrawHours=0;
int DrawMinutes=0;
int DrawSeconds=0;
  u8g.setFont(rus6x12);
//u8g.setFont(u8g_font_fixed_v0);  
//вывод названия родительского меню вверху экрана
u8g.setPrintPos(5, 9);
u8g.print(MenuNames[MenuParent[MenuNowPos-MenuDrawPos]]);
u8g.drawLine( 0, 10, 123,10);
//переменная для вывода пунктов меню на экран
int DrawI=0;
//цикл для вывода пунктов меню на экран
for(DrawI=0; DrawI<MenuDrawCount;DrawI++)  
  {
    u8g.setPrintPos(5, 21+10*DrawI);
    if ((MenuChildFirst[MenuParent[MenuNowPos]]<=(MenuNowPos-MenuDrawPos+DrawI)) and 
      (MenuChildEnd[MenuParent[MenuNowPos]]>=(MenuNowPos-MenuDrawPos+DrawI)))
      { 
        u8g.print(MenuNames[MenuNowPos-MenuDrawPos+DrawI]);  
     
     
        u8g.setPrintPos(80, 21+10*DrawI);
        //Если целое число
        if (MenuTypeCode[MenuNowPos-MenuDrawPos+DrawI]==1) 
          {
            u8g.print(MenuValue[MenuNowPos-MenuDrawPos+DrawI]);  
          }
        
        //Если тип временной интервал  
        if (MenuTypeCode[MenuNowPos-MenuDrawPos+DrawI]==2) 
          {
            DrawHours=MenuValue[MenuNowPos-MenuDrawPos+DrawI] / 3600;
            DrawMinutes=(MenuValue[MenuNowPos-MenuDrawPos+DrawI] % 3600) / 60;
            DrawSeconds=(MenuValue[MenuNowPos-MenuDrawPos+DrawI] % 3600) % 60;
            u8g.print((String)DrawHours+":"+(String)DrawMinutes+":"+(String)DrawSeconds);
          }  
        //Если пункт меню бинарный
        if (MenuTypeCode[MenuNowPos-MenuDrawPos+DrawI]==3) 
          { 
            if (MenuValue[MenuNowPos-MenuDrawPos+DrawI]==0) {u8g.print("Off");}
              else {u8g.print("On");}  
          }     
      }
  }
  
  //если параметр сейчас не редактируется то отображение рамки вокруг выделенного пункта меню
  if (MenuEdit==0)
    {
      
    //u8g.drawLine( 3, 12+10*MenuDrawPos, 70, 12+10*MenuDrawPos);
    //u8g.drawLine(70, 12+10*MenuDrawPos, 70,22+10*MenuDrawPos);
    //u8g.drawLine( 3,22+10*MenuDrawPos, 70,22+10*MenuDrawPos);
    //u8g.drawLine( 3,  22+10*MenuDrawPos, 3,12+10*MenuDrawPos);
    }  
    
  //если параметр сейчас редактируется то отображение рамки вокруг значения параметра
  if (MenuEdit==1)
    {
      
      u8g.drawLine( 75, 12+10*MenuDrawPos, 122, 12+10*MenuDrawPos);
      u8g.drawLine(122, 12+10*MenuDrawPos, 122,22+10*MenuDrawPos);
      u8g.drawLine( 75,22+10*MenuDrawPos, 122,22+10*MenuDrawPos);
      u8g.drawLine( 75,  22+10*MenuDrawPos, 75,12+10*MenuDrawPos);
    }  
}*/


void Draw()
{
  u8g.firstPage();   
  do 
    { 
      //прорисовка статуса калибровки
      if(MenuNowPos!=10 && MenuNowPos!=11 && MenuNowPos!=12 && MenuNowPos!=13 && MenuNowPos!=14 && MenuNowPos!=15 && MenuNowPos!=16 && MenuNowPos!=17 && MenuNowPos!=18 && MenuNowPos!=19 && MenuNowPos!=20)
        DrawMenu();
      else
        DrawSinglePageMenu();
    } while( u8g.nextPage() );
}


//при завершении редактирования пункта меню происходит обновление настроек
void UpdateSettings(bool clearMeenu = false)
{
  if(!clearMeenu){
  //здесь происходит обновление настроек
    EEPROM.write(MenuNowPos*2-2, lowByte(MenuValue[MenuNowPos]));
    EEPROM.write(MenuNowPos*2-1,highByte(MenuValue[MenuNowPos]));
  }
  else
  {
    EEPROM.write(MenuNowPos*2-2, lowByte(0));
    EEPROM.write(MenuNowPos*2-1,highByte(0));

    int i=0;
    for (i=0;i<50;i++){
      MenuValue[i]=(EEPROM.read(i*2-1) << 8);
      MenuValue[i]=MenuValue[i]+(EEPROM.read(i*2-2));
    }
  }
  
}


//Процедура для обработки нажатия кнопки "вверх"
void UpPress()
{
  //если не находимся в режиме редактирования то кнопка используется для передвижения по меню
  if (MenuEdit==0)
  {
    //если текущая позиция в меню больше чем позиция первого элемента в этом меню то можно осуществлять передвижение.
    if (MenuChildFirst[MenuParent[MenuNowPos]]<MenuNowPos)
      {
        //осуществляем передвижение по меню на 1 пункт
        MenuNowPos=MenuNowPos-1;
        //при движении вверх проверяем где расположен выделенный пункт меню на дисплее
        //если выделенный пункт не упирается в край дисплея то также смещаем его на дисплее на 1 позицию
        if (MenuDrawPos>0)
          {
          MenuDrawPos=MenuDrawPos-1;  
          }
      }
  }

  //Если находимся в режиме редактирования
  if (MenuEdit==1)
    {
      //проверяем какого типа меню и проверяем соответствующие ограничения, также контроллируем в зависимости от значения приращение
      //или уменьшение значения
      //Если тип целое число то максимального ограничения нет (добавить потом чтоб бралось максимальное значение из меню)
      if (MenuTypeCode[MenuNowPos]==1) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;
        }
      //Если тип временной интервал  
      if (MenuTypeCode[MenuNowPos]==2) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;
        }
      //Если пункт меню бинарный то инвертируем значение
      if (MenuTypeCode[MenuNowPos]==3) 
        {
          MenuValue[MenuNowPos]=(MenuValue[MenuNowPos]+1) % 2;  
        }
      if (MenuTypeCode[MenuNowPos]==5) 
        {
          MenuValue[MenuNowPos]=(MenuValue[MenuNowPos]+1) % 2;  
        }
      if (MenuTypeCode[MenuNowPos]==7) 
        {
          if(MenuValue[MenuNowPos]==2) 
            MenuValue[MenuNowPos]=0;
          else
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;  
        }
      if (MenuTypeCode[MenuNowPos]==8) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;
        }
      if (MenuTypeCode[MenuNowPos]==9) 
        {
          if(MenuValue[MenuNowPos]==2) 
            MenuValue[MenuNowPos]=0;
          else
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;
        }
      if (MenuTypeCode[MenuNowPos]==10) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]+1;  
        }  
    }
}

// обработать события нажатия новіх кнопок:









//Процедура для обработки нажатия кнопки "вниз"
void DownPress()
{
  //если не находимся в режиме редактирования
  if (MenuEdit==0)
  {
    //проверяем не является ли текущий пункт последним дочерним элементом
    if (MenuChildEnd[MenuParent[MenuNowPos]]>MenuNowPos)
      {
        //если не является то двигаемся на 1 пункт вниз
        MenuNowPos=MenuNowPos+1;
        //проверяем упираемся ли мы в край экрана. максимальное число элементов меню на экране задано переменной MenuDrawCount
        if ((MenuDrawPos<MenuDrawCount-1) and (MenuDrawPos<MenuChildEnd[MenuParent[MenuNowPos]]-MenuChildFirst[MenuParent[MenuNowPos]]))
          {
          //если в край экрана не упираемся то также сдвигаем позицию на экране на 1 пункт вниз
          MenuDrawPos=MenuDrawPos+1;  
          }
      }
  }
    //если находимся в режиме редактирования 
  if (MenuEdit==1)
    {
      
      //проверяем какого типа меню и проверяем соответствующие ограничения, также контроллируем в зависимости от значения приращение
      //или уменьшение значения
      if (MenuTypeCode[MenuNowPos]==1) 
        {
          if (MenuValue[MenuNowPos]>0)
            {
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;
            }
        }
      //Если тип временной интервал  
      if (MenuTypeCode[MenuNowPos]==2) 
        {
          if (MenuValue[MenuNowPos]>0)
            {
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;
            }
        }
      //Если пункт меню бинарный то инвертируем значение
      if (MenuTypeCode[MenuNowPos]==3) 
        {
          MenuValue[MenuNowPos]=(MenuValue[MenuNowPos]+1) % 2;  
        }
      if (MenuTypeCode[MenuNowPos]==5) 
        {
          MenuValue[MenuNowPos]=(MenuValue[MenuNowPos]+1) % 2;  
        }
      if (MenuTypeCode[MenuNowPos]==7) 
        {
          if(MenuValue[MenuNowPos]==0) 
            MenuValue[MenuNowPos]=2;
          else
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;  
        }
        
      if (MenuTypeCode[MenuNowPos]==8) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;
        }
      if (MenuTypeCode[MenuNowPos]==9) 
        {
          if(MenuValue[MenuNowPos]==0) 
            MenuValue[MenuNowPos]=2;
          else
            MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;
        }
      if (MenuTypeCode[MenuNowPos]==10) 
        {
          MenuValue[MenuNowPos]=MenuValue[MenuNowPos]-1;  
        }   
    }  
}  
  
  
//Процедура для обработки нажатия кнопки "влево"  
void LeftPress()
{
  //если не находимся в режиме редактирования
    if (MenuEdit==0)
      { 
        //если пункт меню содержит ненулевой индекс родителя (т.е. мы находимся внутри другого меню)
        if (MenuParent[MenuNowPos]>0)
          { 
            //то переходим на индекс родительского пункта меню
            MenuNowPos=MenuParent[MenuNowPos];
            //установка позиции на экарне, если количество пунктов меньше чем влезает на экране то выделенный пункт будет в самом низу но не в конце
            //иначе будет в самом конце
            if (MenuChildEnd[MenuParent[MenuNowPos]]-MenuChildFirst[MenuParent[MenuNowPos]]<MenuDrawCount)
                MenuDrawPos=MenuNowPos-MenuChildFirst[MenuParent[MenuNowPos]];
            else 
                  MenuDrawPos=MenuDrawCount-1;
          }
      }
    //если находимся в режиме редактирования то выключаем режим редактирования  
    if (MenuEdit==1)
      {  
      MenuEdit=0;
      //запускаем процедуру для обновления настроек. это необходимо для того чтобы параметры меню которые мы поменяли начали действовать
      //в программе для которой мы используем это меню
      UpdateSettings();
      }
}
  
  
//Процедура для обработки нажатия кнопки "вправо"   
void RightPress()
{
  //если код типа элемента отличается от нуля (т.е. выделенный элемент является параметром) то включаем режим редактирования
  if (MenuTypeCode[MenuNowPos]>0 && MenuTypeCode[MenuNowPos]!=5 && MenuTypeCode[MenuNowPos]!=6) //переходим в режим редактированния если тип не равен (источник питанния или значение напряжения)
    {
      MenuEdit=1;  
    }
  //если код типа элемента равен нулю значит в данный момент выделен пункт меню и мы можем войти в него  
  if (MenuTypeCode[MenuNowPos]==0)
    {
    
    //обнуляем позицию выделенного пункта на экране
    MenuDrawPos=0;  
    //переходим на первый дочерний элемент для текущего элемента
    MenuNowPos=MenuChildFirst[MenuNowPos];
    }

   /*if (MenuTypeCode[MenuNowPos]==0)
    {
      //обнуляем позицию выделенного пункта на экране
      MenuDrawPos=2;  
      //переходим на первый дочерний элемент для текущего элемента
      MenuNowPos=MenuChildFirst[MenuNowPos];
    }
     */ 
}

void ControlLed(uint8_t manual=2)
{
  if(digitalRead(brightDisplayPin)==HIGH && manual==2)
  {
    LedFlag =0;
    digitalWrite(brightDisplayPin, LOW);
  }
  else if(digitalRead(brightDisplayPin)==LOW &&manual==2)
  {
    LedFlag = 1;
    digitalWrite(brightDisplayPin, HIGH);
  }
  else
  {
    digitalWrite(brightDisplayPin, manual);
    LedTime1=micros(); 
  }
}

// обработать события нажатия кнопки:
void keypadEvent(KeypadEvent key){
  //Serial.println(key);
  
  KeyState state = keypad.getState();

  if (state == HOLD){
    switch (key){
        case '2':
          UpPress(); 
        break;
        case '8': 
          DownPress();
        break;
  }
  }
  if (state == PRESSED) {
    switch (key){
        case '2':
          UpPress();
          ControlLed(1); 
          break;
        case '8': 
          DownPress();
          ControlLed(1);
          break;
        case '4':
          LeftPress(); 
          ControlLed(1);
          break;
        case '6': 
          RightPress();
          ControlLed(1);
          break;
        case '*': ControlLed();
        break;
        case 'A':
          MenuNowPos=4;
          MenuDrawPos=0;
          ControlLed(1);
        break;
        case 'B': 
          MenuNowPos=10;
          MenuDrawPos=0;
          ControlLed(1);
        break;
        case 'C': 
          MenuNowPos=16;
          MenuDrawPos=0;
          ControlLed(1);
        break;
      }
  }

  
  /*switch (keypad.getState()){
    case PRESSED:
      Serial.println("released");
      Serial.println("released="+ key );
      if(key == '2')
      {
        UpPress();
      }
      else if(key == '4')
      {
        LeftPress();
      }
      switch (key){
        case '2':UpPress(); 
        break;
        case '8': DownPress();
        break;
        case '4':LeftPress(); 
        break;
        case '6': RightPress();
        break;
        case '*': LedControl();
        break;
        case 'C': UpdateSettings(true);
        break;
      }
    
    case HOLD:
        switch (key){
        case '2':
        if (MenuEdit==1)
          UpPress(); 
        break;
        case '8': 
        if (MenuEdit==1)
          DownPress();
        break;
        //case '*': blink = true; break;
      }*/
    //break;
  }




void Start()
{

  
}
void  timerInterruptMoved() {
 
  /*MotorMoved.control();// управвление двигателем
  MotorExtruder.control();
  timeCounterMoved++; // счетчик времени
  
 timeCounterExtruder++;

 if(timeCounterExtruder > 500)
  {
    timeCounterExtruder=0;
    kount++;
    MenuValue[13]=kount;
  }

  if (MotorExtruder.readSteps() > 1000)
    MotorExtruder.step(30000);
  if (MotorMoved.readSteps()  > 1000)
    MotorMoved.step(30000);*/
}
/*void ledDigitDisplay(float num, float time)
{
  unsigned long ltime = millis();
  // Настройки
  // 6, 8, 9, 12 - GND
  int pin[] = {22, 24, 26, 28, 30, 32, 33, 31, 29, 27, 25, 23}; // Пины
  int settingsSegments[] = {pin[10], pin[6], pin[3], pin[1], pin[0], pin[9], pin[4], pin[2]}; // Порядок сегментов
  int segments[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111, 0b10000000, 0b01000000}; // 1, 2, 3, 4, 5, 6, 7, 8, 9, '.', '-'
  for(int i = 0; i < 12; ++i) 
    pinMode(pin[i], OUTPUT); // Определяем пины как выход
  int floatingPoint = 0, minus = 4;
 
 if(num > -1000 && num < 0) // Разбираемся с отрицательными числами
  {
    minus--;
    if(num > -100) minus--;
    if(num > -10) minus--;
   num = -num;
 }
 
 for(int i = 0; num < 1000 && minus == 4; ++i) // Разбираемся с дробными числами
  {
    if(int(num * 10) != int(num)*10)
    {
      floatingPoint++;
      num *= 10;
    }
   else
      break;
  }
 
  for(int i = 0, temp; millis() - ltime <= time * 1000; i++)
 {
  if(i == 4) i = 0;
   temp = int(num / pow(10, i)) % 10; // Цифра которую передадим индикатору
    if(num >= 10000 || num <= -10000 || minus == i) // Если минус или переполнение, передаем '-'
      temp = 11;
 
   if(i == 2 && (num >= 100 || floatingPoint == i || minus == i)) 
    pinMode(pin[11], OUTPUT); 
   else 
    pinMode(pin[11], INPUT);
   if(i == 1 && (num >= 10 || floatingPoint == i || minus == i)) 
    pinMode(pin[8], OUTPUT); 
   else 
    pinMode(pin[8], INPUT); // Работаем с 3 разрядом
   if(i == 0 && (num >= 1 || floatingPoint == i || minus == i)) 
    pinMode(pin[7], OUTPUT); 
   else 
    pinMode(pin[7], INPUT); // Работаем с 2 разрядом

 
   for(int j = 0; j < 8; j++) // Передаем число
     if(segments[temp] & (1 << j))
       digitalWrite(settingsSegments[j], HIGH);
 
    if(floatingPoint && floatingPoint == i) // Передаем точку
     digitalWrite(settingsSegments[7], HIGH);
 
    delay(1); // Небольшая пауза, чтобы светодиоды разгорелись
 
    for(int j = 0; j < 8; j++) digitalWrite(settingsSegments[j], LOW); // Выключаем все светодиоды
 }
}*/

/*# define BEDTEMPTABLE_LEN (sizeof(temptable_11)/sizeof(*temptable_11))
/#define PGM_RD_W(x)   (short)pgm_read_word(&x)
static float analog2tempBed(int raw) {
  float celsius = 0;
  byte i;
  for (i = 1; i < BEDTEMPTABLE_LEN; i++)
    {
      if (PGM_RD_W(temptable_11[i][0]) > raw)
        {
            celsius = PGM_RD_W(temptable_11[i - 1][1]) +
                (raw - PGM_RD_W(temptable_11[i - 1][0])) *
                (float)(PGM_RD_W(temptable_11[i][1]) - PGM_RD_W(temptable_11[i - 1][1])) /
                (float)(PGM_RD_W(temptable_11[i][0]) - PGM_RD_W(temptable_11[i - 1][0]));
            break;
        }
    }

    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN) celsius = PGM_RD_W(temptable_11[i - 1][1]);

    return celsius;
}*/
/*void ledDigitDisplay1(float num, float time)
{
  unsigned long ltime = millis();
  int pin[] = {34, 36, 38, 40, 42, 44, 45, 43, 41, 39, 37, 35}; // Пины
  int settingsSegments[] = {pin[10], pin[6], pin[3], pin[1], pin[0], pin[9], pin[4], pin[2]}; // Порядок сегментов
  int segments[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111, 0b10000000, 0b01000000}; // 1, 2, 3, 4, 5, 6, 7, 8, 9, '.', '-'
  
  for(int i = 0; i < 12; ++i)
    pinMode(pin[i], OUTPUT); 
    
  int floatingPoint = 0, minus = 4;
  if(num > -1000 && num < 0){
    minus--;
    if(num > -100)
      minus--;
    if(num > -10)
      minus--;
    num = -num;}
 
  for(int i = 0; num < 1000 && minus == 4; ++i){
    if(int(num * 10) != int(num)*10){
      floatingPoint++;
      num *= 10;}
    else
      break;}
 
  for(int i = 0, temp; millis() - ltime <= time * 1000; i++){
    if(i == 4) i = 0;
      temp = int(num / pow(10, i)) % 10; // Цифра которую передадим индикатору
    if(num >= 10000 || num <= -10000 || minus == i) // Если минус или переполнение, передаем '-'
      temp = 11;
    if(i == 2 && (num >= 100 || floatingPoint == i || minus == i)) 
      pinMode(pin[11], OUTPUT); 
    else 
      pinMode(pin[11], INPUT); // Работаем с 4 разрядом
    if(i == 1 && (num >= 10 || floatingPoint == i || minus == i)) 
      pinMode(pin[8], OUTPUT); 
      else pinMode(pin[8], INPUT); // Работаем с 3 разрядом
    if(i == 0 && (num >= 1 || floatingPoint == i || minus == i)) 
      pinMode(pin[7], OUTPUT); 
    else 
      pinMode(pin[7], INPUT); 
    for(int j = 0; j < 8; j++) // Передаем число
     if(segments[temp] & (1 << j))
       digitalWrite(settingsSegments[j], HIGH);
 
    if(floatingPoint && floatingPoint == i) // Передаем точку
      digitalWrite(settingsSegments[7], HIGH);
    delay(1); // Небольшая пауза, чтобы светодиоды разгорелись
    for(int j = 0; j < 8; j++) 
      digitalWrite(settingsSegments[j], LOW);}
}*/



//Насос котла
bool ControlBoilerPumpRelay()
{
   
    if(MenuValueSecond[4]<MenuValue[10]) //если температура датчика бойлера менее указанной в настройках
      {
        MenuValue[4]=0; // ставим "Выкл"
        boilerPumpRelay.setPWM(0); //убираем сигнал с реле
      }
    else
      {
        MenuValue[4]=1; // ставим "Вкл"
        boilerPumpRelay.setPWM(255); //максимальный сигнал на реле
      }
    return true;
  

}

//Насос теплоаккумулятора
bool HeatAccumPumpRelay(bool powerLow = false)
  {
    if((MenuValue[18]==2) && (MenuValue[19]==2)) //Если оба режима - "отключено"
    {
      MenuValue[5]=0;
      heatAccumPumpRelay.setPWM(0);
    }
    else if(MenuValueSecond[5]<MenuValue[11]) //если температура датчика насоса теплоаккумулятора меньше значенния указаного в настройках
    {
      MenuValue[5]=0;
      heatAccumPumpRelay.setPWM(0);
    }
    else if(!powerLow) //если не подан флаг отсутсвия сети
    {
      MenuValue[5]=1;
      heatAccumPumpRelay.setPWM(255);
    }
    else
    {
      MenuValue[5]=0;
      heatAccumPumpRelay.setPWM(0);
    }
    return true;
  
}

//Насос циркуляции первичного круга
bool PrimaryCirculationPumpRelay(bool powerLow = false)
{
  
    if((MenuValue[18]==2) && (MenuValue[19]==2)) //Если оба режима - "отключено"
    {
      MenuValue[6]=0;
      primaryCirculationPumpRelay.setPWM(0);
    }
    else if(MenuValueSecond[5]<MenuValue[11]) //если температура датчика теплоаккумулятора меньше значенния указаного в настройках
    {
      MenuValue[6]=0;
      primaryCirculationPumpRelay.setPWM(0);
    }
    else if(!powerLow) //если не подан флаг отсутсвия сети
    {
      MenuValue[6]=1;
      primaryCirculationPumpRelay.setPWM(255);
    }
    else
    {
      MenuValue[6]=0;
      primaryCirculationPumpRelay.setPWM(0);
    }
    return true;
  
}

//Насос циркуляции отопления
bool HeatingCirculationPumpRelay(bool powerLow = false)
{
   
    if(MenuValue[18]==2) //Если отопление отключено
    {
      heatingCirculationPumpRelay.setPWM(0);
      MenuValue[7]=0;
    }
    else if(MenuValueSecond[5]<MenuValue[11]) //если температура датчика теплоаккумулятора меньше значенния указаного в настройках
    {
      heatingCirculationPumpRelay.setPWM(0);
      MenuValue[7]=0;
    }
    else if(!powerLow) //если не подан флаг отсутсвия сети
    {
      heatingCirculationPumpRelay.setPWM(255);
      MenuValue[7]=1;
    }
    else
    {
      heatingCirculationPumpRelay.setPWM(0);
      MenuValue[7]=0;
    }
    return true;
  
}

//Насос циркуляции подачи на теплообменник
bool HeatExchangerPumpRelay()
{
    
    if((MenuValue[19]==2))
    {
      MenuValue[8]=0;
      heatExchangerPumpRelay.setPWM(0);
    }
    else if(MenuValueSecond[5]<MenuValue[11])
    {
      MenuValue[8]=0;
      heatExchangerPumpRelay.setPWM(0);
    }
    else if((MenuValueSecond[6]-MenuValueSecond[9])<MenuValue[12] && (MenuValue[19] == 0))
    {
      MenuValue[8]=0;
      heatExchangerPumpRelay.setPWM(0);
    }
    else if(MenuValue[16]==1)
    {
      MenuValue[8]=0;
      heatExchangerPumpRelay.setPWM(0);
    }
    else
    {
      MenuValue[8]=1;
      heatExchangerPumpRelay.setPWM(255);
    }
    return true;
 
}

//Насос циркуляции горячей воды
bool HeatWaterCirculationPumpRelay()
{
    /*Serial.println("=================================="); 
    Serial.println("MenuValue[19]"); 
    Serial.println(MenuValue[19]);
    Serial.println("Module");
    Serial.println(abs(MenuValueSecond[4]-MenuValueSecond[9])); 
    Serial.println("MenuValueSecond[5]");
    Serial.println(MenuValueSecond[5]); 
    Serial.println("MenuValue[11]");
    Serial.println(MenuValue[11]); 
    Serial.println("MenuValue[12]");
    Serial.println(MenuValue[12]);*/
   
    if((MenuValue[19]==2))// 
    {
      MenuValue[9]=0;
      heatWaterCirculationPumpRelay.setPWM(0);
      //Serial.println("1"); 
    }
    else if((MenuValueSecond[5]<MenuValue[11]) && (MenuValue[19]==0)) //0- горячая вода, 1 - бассейн, 2 - выключено
    {
      MenuValue[9]=0;
      heatWaterCirculationPumpRelay.setPWM(0);
      //Serial.println("2"); 
    }
    else if((MenuValueSecond[6]-MenuValueSecond[9])<MenuValue[12] && (MenuValue[19] == 0))
    {
      MenuValue[9]=0;
      heatWaterCirculationPumpRelay.setPWM(0); 
    }
    else if(MenuValue[16]==1)
    {
      MenuValue[9]=0;
      heatWaterCirculationPumpRelay.setPWM(0);
    }
    else
    {
      MenuValue[9]=1;
      heatWaterCirculationPumpRelay.setPWM(255); 
    }
    return true;
  
}

bool CheckTemparatureAllSensors(){
  if(MenuValueSecond[4]<MenuValue[15] or MenuValueSecond[5]<MenuValue[15] or MenuValueSecond[6]<MenuValue[15] or MenuValueSecond[9]<MenuValue[15])
    return true;
  else
    return false;
}

void DangerMode(bool activate = false)
{
  if(activate)
    {
      boilerPumpRelay.setPWM(255);
      heatAccumPumpRelay.setPWM(255);
      primaryCirculationPumpRelay.setPWM(255);
      heatingCirculationPumpRelay.setPWM(255);
      heatExchangerPumpRelay.setPWM(255);
      heatWaterCirculationPumpRelay.setPWM(255);
    }
}



void ControlRelay()
{
  if(CheckTemparatureAllSensors())
  {
    DangerMode(true);
  }
  else{
    
    //Насос котла
    ControlBoilerPumpRelay();
  
  
    //14 - сколько стоим
    //13 - сколько работаем 
  
    if(MenuValue[16]==1 && MenuValue[18]==0){
      RelayEconomTime2 = micros();
      if ((RelayEconomTime2-RelayEconomTime1)>RelayEconomInterval && RalayFlag==false) 
      {
        RelayEconomTime1=RelayEconomTime2;
        //Насос теплоаккумулятора
        HeatAccumPumpRelay();
    
        //Насос циркуляции первичного круга
        PrimaryCirculationPumpRelay();
  
        //Насос циркуляции отопления
        HeatingCirculationPumpRelay();
  
        RalayFlag = true;
      }
      else if((RelayEconomTime2-RelayEconomTime1)>RelayPowerInterval && RalayFlag==true)
      {
        RelayEconomTime1=RelayEconomTime2;
        //Насос теплоаккумулятора
        HeatAccumPumpRelay(true);
    
        //Насос циркуляции первичного круга
        PrimaryCirculationPumpRelay(true);
  
        //Насос циркуляции отопления
        HeatingCirculationPumpRelay(true);
  
        RalayFlag = false;
      }
    }
    else{
       //Насос теплоаккумулятора
      HeatAccumPumpRelay();
      
      //Насос циркуляции первичного круга
      PrimaryCirculationPumpRelay();
    
      //Насос циркуляции отопления
      HeatingCirculationPumpRelay();
    }
     
    //Насос циркуляции подачи на теплообменник
    HeatExchangerPumpRelay();
    
    //Насос циркуляции горячей воды
    HeatWaterCirculationPumpRelay();
  }

}

void ControlPower()
{
  uint16_t countPWM = analogRead(powerLoadPin);
  
  float peremen = ((float)countPWM/1024)*50;
  MenuValue[17]=round(peremen);
  if(countPWM < 800)
  {
    powerLoadCounter++;
    MenuValue[16]=1;
    PowerBatteryTime2 = millis();
  }
  else
  {
    powerLoadCounter=0;
    MenuValue[16]=0;
    PowerBatteryTime1 =millis();
  }
}


void ControlRelayTick()
{ 
  boilerPumpRelay.tick();
  heatAccumPumpRelay.tick();
  primaryCirculationPumpRelay.tick();
  heatingCirculationPumpRelay.tick(); 
  heatExchangerPumpRelay.tick();
  heatWaterCirculationPumpRelay.tick(); 
}


void loop()
{
  //
  RelayEconomInterval = (MenuValue[14]*1000000*60);
  RelayPowerInterval = (MenuValue[13]*1000000*60);
  
  
  //analogWrite(16, 150);
  //uiStep();
  //проверка таймера для обработки графики, аналогично с обработкой клавиатуры 
  //обновление графики происходит не чаще заданного интервала DrawTimeInterval
  //temp_extruder_1 = analogRead(A0);
  //temp_c_extruder_1= analog2tempBed(temp_extruder_1);
  //int temp_c_extruder_int_1 = temp_c_extruder_1 + 0.6;
  //temp_extruder_2 = analogRead(A2);
  //temp_c_extruder_2= analog2tempBed(temp_extruder_2);
  //int temp_c_extruder_int_2 = temp_c_extruder_2 + 0.6;
 /* if(timeCounterExtruder > 5000)
  {
    timeCounterExtruder=0;
    kount++;
    MenuValue[13]=kount;
  }*/
  //MenuValue[10]=temp_c_extruder_int_1;
  //MenuValue[11]=temp_c_extruder_int_2;
  //MenuValue[13]= timeCounterMoved;
  //randNumber = random(10, 100);
  //MenuValue[9]=randNumber;

  char key = keypad.getKey();
  DrawTime2=micros();
  if ((DrawTime2-DrawTime1)>DrawTimeInterval) 
    {
      DrawTime1=DrawTime2;
      Draw();
      ControlPower();
    }

  TempSensorTime2=micros();
  if ((TempSensorTime2-TempSensorTime1)>TempSensorTimeInterval) 
    {
      TempSensorTime1=TempSensorTime2;
      GetTempValue();
    }

    RelayTime2=micros();
  if ((RelayTime2-RelayTime1)>RelayTimeInterval) 
    {
      RelayTime1=RelayTime2;
      ControlRelay();
    }
    
    LedTime2=micros();
    if ((LedTime2-LedTime1)>LedTimeInterval && MenuValue[16]==1) 
    {
      LedTime1=LedTime2;
      ControlLed(0);
    }
    else if(MenuValue[16]==0)
    {
      ControlLed(LedFlag);
    }
    
    
    ControlRelayTick();
  
    
    //char key = keypad.getKey();

    

    
   // vhod=double(MenuValue[10]);
    //myPID.Compute();
    //temp = MenuValue[10];
   //ledDigitDisplay1(temp2,0.01);
  //int pidd = pidOutput+0.6;
  //analogWrite(first_heat_element, pidd);
  //analogWrite(second_heat_element, pidd);
  //analogWrite(third_heat_element, pidd);
}
