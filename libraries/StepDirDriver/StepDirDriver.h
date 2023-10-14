/*
StepDirDriver.h - библиотека управления STEP/DIR драйвером шагового двигателя

Подробно описана в Уроке 34.
http://mypractic.ru/uroki-programmirovaniya-arduino-navigaciya-po-urokam

Библиотека разработана Калининым Эдуардом
mypractic.ru 
*/

// проверка, что библиотека еще не подключена
#ifndef StepDirDriver_h // если библиотека StepDirDriver не подключена
#define StepDirDriver_h // тогда подключаем ее

#include "Arduino.h"

class StepDirDriver {

  public:
    StepDirDriver(byte pinStep, byte pinDir, byte pinEn); // конструктор
    void  control();  // управление, метод должен вызываться регулярно с максимальной частотой коммутации фаз
    void  step(int steps);  // инициирует поворот двигателя на заданное число шагов
    void  setMode(byte stepMode, boolean fixStop);  // задает режимы коммутации фаз и остановки
    void  setDivider(int divider);  // установка делителя частоты для коммутации фаз
    int readSteps();  // чтение оставшихся шагов

    private:
      int _steps;        // оставшееся число шагов 
      boolean _fixStop;  // признак фиксации положения при остановке
      unsigned int  _divider;  // делитель частоты для коммутации фаз
      unsigned int  _dividerCount;  // счетчик делителя частоты для коммутации фаз
      byte _pinStep, _pinDir, _pinEn;
      int _prevSteps;      
} ;

#endif