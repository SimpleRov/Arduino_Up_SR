#ifndef Type_H_
#define Type_H_

//*******************************  Библиотеки  ******************************//
// Класс Arduino.
#include "Arduino.h"
//******************************* /Библиотеки  ******************************//
typedef struct 
{
  uint8_t bfirst, bsecond, statuswork;
  uint8_t lx, ly, rx, ry;
} ps2Struct;  

typedef struct 
{
  uint8_t errore;
} rovDataStruct; 

#endif /* Type_H_ */
