//*******************************  Библиотеки  ******************************//
// Класс общие типы данных.
#include "Type.h"

#include "Def.h"

#if defined(ARDUINO_PRO_MINI)
  #include <SoftwareSerial.h>
#endif
//*******************************  /Библиотеки  *****************************//

//********************  Объявление переменных, констант  ********************//
// Структура хранящая текущие значения нажатых кнопок и положения стиков джойстика от PS2.
extern ps2Struct ps2S;

// Структура хранящая предыдущие значения нажатых кнопок и положения стиков джойстика от PS2.
extern rovDataStruct rovDataS;
//********************  /Объявление переменных, констант  *******************//

//*******************  Переменные и функции для отладки  ********************//
#if defined(ARDUINO_PRO_MICRO)
  extern Serial_* DebugSerial;
  
  void SetSSerial(Serial_* HWSerial);
#endif

#if defined(ARDUINO_PRO_MINI)
  extern SoftwareSerial* DebugSerial;
  
  void SetSSerial(SoftwareSerial* HWSerial);
#endif
//*******************  /Переменные и функции для отладки  *******************//
