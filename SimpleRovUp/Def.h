#ifndef DEF_H_
#define DEF_H_

//***********************  Определяем тип ардуины  **************************//
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega2560__)
  
  // Частота процессора.
  #ifndef F_CPU
    #define F_CPU 16000000UL   
  #endif                             

  // Определяем тип ардуины.
  #if defined(__AVR_ATmega328P__)
    // Если ArduinoProMini.
    #define ARDUINO_PRO_MINI
  #endif
  #if defined(__AVR_ATmega32U4__)
    // Если ArduinoProMicro.
    #define ARDUINO_PRO_MICRO
  #endif
  #if defined(__AVR_ATmega2560__)
    // Если ArduinoMega.
    #define ARDUINO_MEGA_2560
  #endif
#else
  #error This board not support!
#endif
//***********************  /Определяем тип ардуины  *************************//

//***************************  Определение пинов  ***************************//
// Пин Arduino для Playstation joystick линия Dat.
#define PS2_PIN_DAT        4     

// Пин Arduino для Playstation joystick линия Cmd.
#define PS2_PIN_CMD        5  

// Пин Arduino для Playstation joystick линия Sel.
#define PS2_PIN_SEL        6  

// Пин Arduino для Playstation joystick линия Clk.
#define PS2_PIN_CLK        7

// Пин Led для Arduino. 
#define LED_PIN            13 
//*************************** /Определение пинов  ***************************//

#endif /* DEF_H_ */
