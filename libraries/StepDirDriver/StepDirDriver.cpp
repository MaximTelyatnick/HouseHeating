/*
StepDirDriver.h - библиотека управления STEP/DIR драйвером шагового двигателя

Подробно описана в Уроке 34.
http://mypractic.ru/uroki-programmirovaniya-arduino-navigaciya-po-urokam

Библиотека разработана Калининым Эдуардом
mypractic.ru 
*/

#include "Arduino.h"
#include "StepDirDriver.h"

//---------------------------- конструктор -----------------------------------
StepDirDriver::StepDirDriver (byte pinStep, byte pinDir, byte pinEn) {

  // перегрузка номеров выводов
  _pinStep = pinStep;
  _pinDir = pinDir;
  _pinEn = pinEn;
  
  // установка выводов
  pinMode(_pinStep, OUTPUT); digitalWrite(_pinStep, LOW);
  pinMode(_pinDir, OUTPUT); digitalWrite(_pinDir, LOW);
  pinMode(_pinEn, OUTPUT); digitalWrite(_pinEn, HIGH);
  
  // начальное состояние параметров
  _steps= 0;
  _fixStop= false;
  _divider= 100;
  _dividerCount= 0;   
  _prevSteps= 0;
}


//------------------------------- управление коммутацией фаз
// метод должен вызываться регулярно с максимальной частотой коммутации фаз
void  StepDirDriver::control() {
  
  // делитель частоты коммутации
  if ( _steps == 0 ) { 
    // двигатель остановлен
    _dividerCount= 65534;  // сброс делителя частоты
    _prevSteps = 0;
    return;
    }
    // двигатель не остановлен
    _dividerCount++;  
    if ( _dividerCount < _divider ) return;  
      _dividerCount= 0;        
  
  if ( _prevSteps != 0 ) {
    if ( _steps > 0 ) _steps--; // вращение против часовой стрелки
    else              _steps++; // вращение по часовой стрелке           
  }
  
  _prevSteps = _steps;

  if ( _steps != 0 ) {
    // сделать шаг
    digitalWrite(_pinStep, HIGH);
    delayMicroseconds(10);  // 10 мкс
    digitalWrite(_pinStep, LOW);        
  }
  else {
    if ( _fixStop == false ) digitalWrite(_pinEn, HIGH);    
  }          
}


//------------------------------- запуск вращения
// инициирует поворот двигателя на заданное число шагов
void  StepDirDriver::step(int steps) {

  // блокировка
  if ( (steps == 0) && (_fixStop == false) ) digitalWrite(_pinEn, HIGH);
  else digitalWrite(_pinEn, LOW);

  // направление вращения    
  if ( steps < 0 ) digitalWrite(_pinDir, LOW);
  else digitalWrite(_pinDir, HIGH);

  noInterrupts();
  _steps= steps;
  interrupts();  
}

//------------------------------ режим коммутации фаз и остановки
void  StepDirDriver::setMode(byte stepMode, boolean fixStop)  {  
  _fixStop= fixStop;
}

//------------------------------ установка делителя частоты для коммутации фаз
void  StepDirDriver::setDivider(int divider)  {
  _divider= divider;
  //_dividerCount= 0;  
}

//----------------------------- чтение оставшихся шагов
int StepDirDriver::readSteps()  {
  int stp;
  noInterrupts();
  stp= _steps;
  interrupts();
  return(stp);
}
